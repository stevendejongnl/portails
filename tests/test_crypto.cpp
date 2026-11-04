// SPDX-License-Identifier: AGPL-3.0-or-later
#include "crypto.h"

#include <QTest>

using namespace portails;

namespace {

const QByteArray CONTEXT = "tab:one";
const QByteArray PLAIN = "session cookie data";

QByteArray keyFor(const QByteArray &machineId)
{
    return deriveProfileKey(machineId);
}

}  // namespace

class TestCrypto : public QObject {
    Q_OBJECT

private slots:
    void roundTrip()
    {
        const QByteArray blob = encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT);
        QVERIFY(!blob.isEmpty());
        QCOMPARE(decryptBlob(keyFor("machine-a"), blob, CONTEXT).value(), PLAIN);
    }

    void emptyDataRoundTrips()
    {
        const QByteArray blob = encryptBlob(keyFor("machine-a"), {}, CONTEXT);
        QVERIFY(!blob.isEmpty());
        QVERIFY(decryptBlob(keyFor("machine-a"), blob, CONTEXT).has_value());
    }

    void plainTextIsNotVisibleInTheBlob()
    {
        QVERIFY(!encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT).contains(PLAIN));
    }

    void anotherMachineCannotOpenTheBlob()
    {
        const QByteArray blob = encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT);
        QVERIFY(!decryptBlob(keyFor("machine-b"), blob, CONTEXT).has_value());
    }

    void anotherContextCannotOpenTheBlob()
    {
        const QByteArray blob = encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT);
        QVERIFY(!decryptBlob(keyFor("machine-a"), blob, "tab:two").has_value());
    }

    void everyChangedByteIsDetected()
    {
        const QByteArray blob = encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT);
        for (int index = 0; index < blob.size(); ++index) {
            QByteArray changed = blob;
            changed[index] = char(changed[index] ^ 0x01);
            QVERIFY2(!decryptBlob(keyFor("machine-a"), changed, CONTEXT).has_value(), qPrintable(QString::number(index)));
        }
    }

    void truncatedBlobIsRejected()
    {
        const QByteArray blob = encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT);
        QVERIFY(!decryptBlob(keyFor("machine-a"), blob.left(blob.size() - 1), CONTEXT).has_value());
        QVERIFY(!decryptBlob(keyFor("machine-a"), {}, CONTEXT).has_value());
    }

    void everyEncryptionUsesAFreshNonce()
    {
        QVERIFY(encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT) != encryptBlob(keyFor("machine-a"), PLAIN, CONTEXT));
    }

    void wrongKeySizeIsRejected()
    {
        QVERIFY(encryptBlob("short", PLAIN, CONTEXT).isEmpty());
        QVERIFY(!decryptBlob("short", "PTV1", CONTEXT).has_value());
    }

    void keyringSecretChangesTheKey()
    {
        const QByteArray secret(32, 'k');
        const QByteArray mixed = deriveProfileKey("machine-a", secret);
        QCOMPARE(mixed.size(), KEY_SIZE);
        QCOMPARE(mixed, deriveProfileKey("machine-a\n", secret));
        QVERIFY(mixed != keyFor("machine-a"));
        QVERIFY(mixed != deriveProfileKey("machine-b", secret));
        QVERIFY(mixed != deriveProfileKey("machine-a", QByteArray(32, 'j')));
        QVERIFY(deriveProfileKey("machine-a", {}).isEmpty());
        QVERIFY(deriveProfileKey("", secret).isEmpty());
    }

    void blobOfTheKeyringKeyDoesNotOpenWithTheMachineKeyAlone()
    {
        const QByteArray mixed = deriveProfileKey("machine-a", QByteArray(32, 'k'));
        const QByteArray blob = encryptBlob(mixed, PLAIN, CONTEXT);
        QVERIFY(!decryptBlob(keyFor("machine-a"), blob, CONTEXT).has_value());
        QVERIFY(decryptBlob(mixed, blob, CONTEXT).has_value());
    }

    void keyDerivation()
    {
        QCOMPARE(keyFor("machine-a").size(), KEY_SIZE);
        QCOMPARE(keyFor("machine-a"), keyFor("machine-a\n"));
        QVERIFY(keyFor("machine-a") != keyFor("machine-b"));
        QVERIFY(keyFor("machine-a") != QByteArray("machine-a"));
        QVERIFY(keyFor("").isEmpty());
    }
};

QTEST_APPLESS_MAIN(TestCrypto)
#include "test_crypto.moc"
