#include "devicemonitor.h"

#include "extern-plugininfo.h"

#include <QNetworkInterface>

DeviceMonitor::DeviceMonitor(const QString &macAddress, const QString &ipAddress, QObject *parent):
    QObject(parent)
{
    m_host = new Host();
    m_host->setMacAddress(macAddress);
    m_host->setAddress(ipAddress);
    m_host->setReachable(false);

    m_arpLookupProcess = new QProcess(this);
    connect(m_arpLookupProcess, SIGNAL(finished(int)), this, SLOT(arpLookupFinished(int)));

    m_pingProcess = new QProcess(this);
    m_pingProcess->setReadChannelMode(QProcess::MergedChannels);
    connect(m_pingProcess, SIGNAL(finished(int)), this, SLOT(pingFinished(int)));
}

DeviceMonitor::~DeviceMonitor()
{
    delete m_host;
}

void DeviceMonitor::update()
{
    if (m_pingProcess->state() != QProcess::NotRunning) {
        qCDebug(dcNetworkDetector()) << "Previous ping still running for device" << m_host->address() << ". Not updating.";
        return;
    }
    lookupArpCache();
}

void DeviceMonitor::lookupArpCache()
{
    m_arpLookupProcess->start("ip", {"-4", "-s", "neighbor", "list"});
}

void DeviceMonitor::ping()
{
    qCDebug(dcNetworkDetector()) << "Sending ARP Ping to" << m_host->hostName() << m_host->macAddress() << m_host->address();
    QNetworkInterface targetInterface;
    foreach (const QNetworkInterface &interface, QNetworkInterface::allInterfaces()) {
        foreach (const QNetworkAddressEntry &addressEntry, interface.addressEntries()) {
            QHostAddress clientAddress(m_host->address());
            if (clientAddress.isInSubnet(addressEntry.ip(), addressEntry.prefixLength())) {
                targetInterface = interface;
                break;
            }
        }
    }
    if (!targetInterface.isValid()) {
        qCWarning(dcNetworkDetector()) << "Could not find a suitable interface to ping for" << m_host->address();
        if (m_host->reachable()) {
            m_host->setReachable(false);
            emit reachableChanged(false);
        }
        return;
    }

    m_pingProcess->start("arping", {"-I", targetInterface.name(), "-f", "-w", "90", m_host->address()});
}

void DeviceMonitor::arpLookupFinished(int exitCode)
{
    if (exitCode != 0) {
        qCWarning(dcNetworkDetector()) << "Error looking up ARP cache.";
        return;
    }

    QString data = QString::fromLatin1(m_arpLookupProcess->readAll());
    bool found = false;
    bool needsPing = true;
    QString mostRecentIP = m_host->address();
    qlonglong secsSinceLastSeen = -1;
    foreach (QString line, data.split('\n')) {
        line.replace(QRegExp("[ ]{1,}"), " ");
        QStringList parts = line.split(" ");
        int lladdrIndex = parts.indexOf("lladdr");
        if (lladdrIndex >= 0 && parts.count() > lladdrIndex + 1 && parts.at(lladdrIndex+1).toLower() == m_host->macAddress().toLower()) {
            found = true;
            QString entryIP = parts.first();
            if (parts.last() == "REACHABLE") {
                qCDebug(dcNetworkDetector()) << "Device" << m_host->macAddress() << "found in ARP cache and claims to be REACHABLE";
                if (!m_host->reachable()) {
                    m_host->setReachable(true);
                    emit reachableChanged(true);
                }
                m_host->seen();
                emit seen();
                // Verify if IP address is still the same
                if (entryIP != mostRecentIP) {
                    mostRecentIP = entryIP;
                }
                // If we have a reachable entry, stop processing here
                needsPing = false;
                m_failedPings = 0;
                break;
            } else {
                // ARP claims the device to be stale... Flagging device to require a ping.
                qCDebug(dcNetworkDetector()) << "Device" << m_host->macAddress() << "found in ARP cache with IP" << entryIP << "but is marked as" << parts.last();

                int usedIndex = parts.indexOf("used");
                if (usedIndex >= 0 && parts.count() > usedIndex + 1) {
                    QString usedFields = parts.at(usedIndex + 1);
                    qlonglong newSecsSinceLastSeen = usedFields.split("/").first().toInt();
                    if (secsSinceLastSeen == -1 || newSecsSinceLastSeen < secsSinceLastSeen) {
                        secsSinceLastSeen = newSecsSinceLastSeen;
                        mostRecentIP = entryIP;
                    }
                }
            }
        }
    }
    if (mostRecentIP != m_host->address()) {
        qCDebug(dcNetworkDetector()) << "IP seems to have changed IP:" << m_host->address() << "->" << mostRecentIP;
        m_host->setAddress(mostRecentIP);
        emit addressChanged(mostRecentIP);
    }
    if (!found) {
        qCDebug(dcNetworkDetector()) << "Device" << m_host->macAddress() << "not found in ARP cache.";
        ping();
    } else if (needsPing) {
        ping();
    }
}

void DeviceMonitor::pingFinished(int exitCode)
{
    if (exitCode == 0) {
        // we were able to ping the device
        qCDebug(dcNetworkDetector()) << "Ping successful for" << m_host->macAddress() << m_host->address();
        m_host->seen();
        if (!m_host->reachable()) {
            m_host->setReachable(true);
            emit reachableChanged(true);
        }
        emit seen();
        m_failedPings = 0;
    } else {
        qCDebug(dcNetworkDetector()) << "Could not ping device" << m_host->macAddress() << m_host->address();
        m_failedPings++;
        if (m_failedPings > 3 && m_host->reachable()) {
            m_host->setReachable(false);
            emit reachableChanged(false);
        }
    }
    // read data to discard it from socket
    QString data = QString::fromLatin1(m_pingProcess->readAll());
    Q_UNUSED(data)
//    qCDebug(dcNetworkDetector()) << "have ping data" << data;
}
