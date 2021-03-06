#include "test.h"
#include "ssltests.h"

#include <QCoreApplication>

#ifdef UNSAFE
#include "sslunsafesocket.h"
#else
#include <QSslSocket>
#endif

// Target SslTest is SslTest02:
// "certificate trust test with self-signed certificate for user-supplied common name"

// do not verify peer certificate, send data to socket
// check for proper test result code and intercepted data
class Test01 : public Test
{
    Q_OBJECT
public:
    int getId() { return 1; }

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

    void setSslTest() { targetTest = QString("SslTest02"); sslTest = new SslTest02; }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;
        QByteArray data = QByteArray("GET / HTTP/1.0\r\n\r\n");

        socket->setPeerVerifyMode(XSslSocket::VerifyNone);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            qDebug() << socket->error();
            qDebug() << socket->errorString();

            printTestFailed();
        } else {
            socket->write(data);
            socket->waitForReadyRead();

            // we should wait until test finishes prior to querying for test results
            while (sslTest->result() == SslTest::SSLTEST_RESULT_UNDEFINED)
                QThread::msleep(50);

            if ((sslTest->result() == SslTest::SSLTEST_RESULT_DATA_INTERCEPTED)
                    && (sslTest->interceptedData() == data)) {
                printTestSucceeded();
            } else {
                printTestFailed();
            }
        }
        socket->disconnectFromHost();

        this->deleteLater();
        QThread::currentThread()->quit();
    }

};

// do not verify peer certificate, disconnect after timeout
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

    void setSslTest() { targetTest = QString("SslTest02"); sslTest = new SslTest02; }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyNone);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            qDebug() << socket->error();
            qDebug() << socket->errorString();

            printTestFailed();
        } else {
            QThread::msleep(5500);

            // we should wait until test finishes prior to querying for test results
            while (sslTest->result() == SslTest::SSLTEST_RESULT_UNDEFINED)
                QThread::msleep(50);

            if (sslTest->result() == SslTest::SSLTEST_RESULT_CERT_ACCEPTED) {
                printTestSucceeded();
            } else {
                printTestFailed();
            }
        }
        socket->disconnectFromHost();

        this->deleteLater();
        QThread::currentThread()->quit();
    }

};

// do verify peer certificate
// check for proper test result code
class Test03 : public Test
{
    Q_OBJECT
public:
    int getId() { return 3; }

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

    void setSslTest() { targetTest = QString("SslTest02"); sslTest = new SslTest02; }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyPeer);

        socket->connectToHostEncrypted("localhost", 8443);

        if (!socket->waitForEncrypted()) {
            int res = QString::compare(socket->errorString(),
                                       "The host name did not match any of the valid hosts for this certificate");
            // we should wait until test finishes prior to querying for test results
            while (sslTest->result() == SslTest::SSLTEST_RESULT_UNDEFINED)
                QThread::msleep(50);

            if ((res == 0) && (sslTest->result() == SslTest::SSLTEST_RESULT_SUCCESS)) {
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

// connect to localhost, but set server name to the same as for ssl server
// do verify peer certificate
// check for proper test result code
class Test04 : public Test
{
    Q_OBJECT
public:
    int getId() { return 4; }

    void setTestSettings()
    {
        testSettings.setUserCN("www.example.com");
    }

    void setSslTest() { targetTest = QString("SslTest02"); sslTest = new SslTest02; }

public slots:

    void run()
    {
        XSslSocket *socket = new XSslSocket;

        socket->setPeerVerifyMode(XSslSocket::VerifyPeer);

        socket->connectToHostEncrypted("localhost", 8443, "www.example.com");

        if (!socket->waitForEncrypted()) {
            int res = QString::compare(socket->errorString(),
                                       "The issuer certificate of a locally looked up certificate could not be found");
            // we should wait until test finishes prior to querying for test results
            while (sslTest->result() == SslTest::SSLTEST_RESULT_UNDEFINED)
                QThread::msleep(50);

            if ((res == 0) && (sslTest->result() == SslTest::SSLTEST_RESULT_SUCCESS)) {
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
            << new Test03
            << new Test04
               ;

    while (autotests.size() > 0) {
        launchTest(autotests.takeFirst());
    }

    return 0; //a.exec();
}

#include "tests_SslTest02.moc"
