#include "test.h"
#include "ssltests.h"
#include "ciphers.h"

#include <QCoreApplication>

#ifdef UNSAFE
#include "sslunsafesocket.h"
#else
#include <QSslSocket>
#endif

// Target SslTest is SslTest13:
// "test for TLS 1.0 protocol support"


// do verify peer certificate, use TLSv1.1 and stronger protocols
// check for proper test result code
class Test01 : public Test
{
    Q_OBJECT
public:
    int getId() { return 1; }

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

    void setSslTest() { targetTest = QString("SslTest13"); sslTest = new SslTest13; }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyPeer);
        socket->setProtocol(XSsl::TlsV1_1OrLater);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            // we should wait until test finishes prior to querying for test results
            while (sslTest->result() == SslTest::SSLTEST_RESULT_UNDEFINED)
                QThread::msleep(50);

            if (sslTest->result() == SslTest::SSLTEST_RESULT_SUCCESS) {
                printTestSucceeded();
            } else {
                printTestFailed();
            }
        } else {
            printTestFailed();
        }
        socket->disconnectFromHost();

        this->deleteLater();
        QThread::currentThread()->quit();
    }

};

// do verify peer certificate, use TlsV1_0 protocol
// check for proper test result code
class Test02 : public Test
{
    Q_OBJECT
public:
    int getId() { return 2; }

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

    void setSslTest() { targetTest = QString("SslTest13"); sslTest = new SslTest13; }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyPeer);
        socket->setProtocol(XSsl::TlsV1_0);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            // we should wait until test finishes prior to querying for test results
            while (sslTest->result() == SslTest::SSLTEST_RESULT_UNDEFINED)
                QThread::msleep(50);

            if (sslTest->result() == SslTest::SSLTEST_RESULT_PROTO_ACCEPTED) {
                printTestSucceeded();
            } else {
                printTestFailed();
            }
        } else {
            printTestFailed();
        }
        socket->disconnectFromHost();

        this->deleteLater();
        QThread::currentThread()->quit();
    }
};


void launchTest(Test *autotest)
{
    WHITE(QString("launching autotest #%1").arg(autotest->getId()));

    // we should call it outside of its own thread
    autotest->prepare();

    QThread *autotestThread = new QThread;
    autotest->moveToThread(autotestThread);
    QObject::connect(autotestThread, SIGNAL(started()), autotest, SLOT(run()));
    QObject::connect(autotestThread, SIGNAL(finished()), autotestThread, SLOT(deleteLater()));

    autotestThread->start();

    autotestThread->wait();
}

int main(int argc, char *argv[])
{
    // we need QCoreApplication instance to initialize Qt internals
    QCoreApplication a(argc, argv);

    QList<Test *> autotests = QList<Test *>()
            << new Test01
            << new Test02
               ;

    while (autotests.size() > 0) {
        launchTest(autotests.takeFirst());
    }

    return 0; //a.exec();
}

#include "tests_SslTest13.moc"
