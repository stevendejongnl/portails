// SPDX-License-Identifier: AGPL-3.0-or-later
#include "crypto.h"
#include "profile_vault.h"

#include <QDataStream>
#include <QFile>
#include <QTemporaryDir>
#include <QTest>

using namespace portails;

namespace {

const QByteArray KEY_A = deriveProfileKey("machine-a");
const QByteArray KEY_B = deriveProfileKey("machine-b");

void writeText(const QString &path, const QByteArray &text)
{
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write(text);
}

QByteArray readText(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? file.readAll() : QByteArray();
}

}  // namespace

class TestProfileVault : public QObject {
    Q_OBJECT

private slots:
    void init()
    {
        encrypted = std::make_unique<QTemporaryDir>();
        working = std::make_unique<QTemporaryDir>();
    }

    void sealedProfileComesBackAfterTheWorkingFolderIsCleared()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "sessionKey=secret");
        writeText(vault.storagePath("one") + "/Local Storage/leveldb/000003.log", "token=abc");
        QVERIFY(vault.seal("one"));
        vault.clearWorkingDirectory();

        ProfileVault reopened(encrypted->path(), working->path(), KEY_A);
        QVERIFY(reopened.restore("one"));
        QCOMPARE(readText(reopened.storagePath("one") + "/Cookies"), QByteArray("sessionKey=secret"));
        QCOMPARE(readText(reopened.storagePath("one") + "/Local Storage/leveldb/000003.log"), QByteArray("token=abc"));
    }

    void nothingReadableIsStoredOnDisk()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "sessionKey=secret");
        QVERIFY(vault.seal("one"));
        const QByteArray stored = readText(encrypted->path() + "/one.vault");
        QVERIFY(!stored.isEmpty());
        QVERIFY(!stored.contains("sessionKey"));
        QVERIFY(!stored.contains("secret"));
    }

    void blobFileIsOnlyReadableByTheOwner()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "x");
        QVERIFY(vault.seal("one"));
        const auto permissions = QFile::permissions(encrypted->path() + "/one.vault");
        QVERIFY(!(permissions & (QFile::ReadGroup | QFile::ReadOther | QFile::WriteGroup | QFile::WriteOther)));
    }

    void anotherMachineStartsLoggedOutAndTheOldFileIsDeleted()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "sessionKey=secret");
        QVERIFY(vault.seal("one"));
        vault.clearWorkingDirectory();

        ProfileVault otherMachine(encrypted->path(), working->path(), KEY_B);
        QVERIFY(!otherMachine.restore("one"));
        QVERIFY(!QFile::exists(encrypted->path() + "/one.vault"));
        QVERIFY(!QFile::exists(otherMachine.storagePath("one") + "/Cookies"));
    }

    void fileOfAnotherTabIsRejected()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "sessionKey=secret");
        QVERIFY(vault.seal("one"));
        QVERIFY(QFile::copy(encrypted->path() + "/one.vault", encrypted->path() + "/two.vault"));
        QVERIFY(!vault.restore("two"));
    }

    void changedFileIsRejected()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "sessionKey=secret");
        QVERIFY(vault.seal("one"));
        QByteArray stored = readText(encrypted->path() + "/one.vault");
        stored[stored.size() / 2] = char(stored[stored.size() / 2] ^ 0x01);
        writeText(encrypted->path() + "/one.vault", stored);
        QVERIFY(!vault.restore("one"));
    }

    void withoutAKeyNothingIsSaved()
    {
        ProfileVault vault(encrypted->path(), working->path(), {});
        QVERIFY(!vault.isSaving());
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "x");
        QVERIFY(!vault.seal("one"));
        QVERIFY(!QFile::exists(encrypted->path() + "/one.vault"));
    }

    void fileOfTheLegacyKeyOpensAndIsSavedWithTheNewKey()
    {
        const QByteArray newKey = deriveProfileKey("machine-a", QByteArray(32, 'k'));
        ProfileVault old(encrypted->path(), working->path(), KEY_A);
        old.restore("one");
        writeText(old.storagePath("one") + "/Cookies", "sessionKey=secret");
        QVERIFY(old.seal("one"));
        old.clearWorkingDirectory();

        ProfileVault upgraded(encrypted->path(), working->path(), newKey, KEY_A);
        QVERIFY(upgraded.restore("one"));
        QCOMPARE(readText(upgraded.storagePath("one") + "/Cookies"), QByteArray("sessionKey=secret"));
        QVERIFY(upgraded.seal("one"));
        upgraded.clearWorkingDirectory();

        ProfileVault newOnly(encrypted->path(), working->path(), newKey);
        QVERIFY(newOnly.restore("one"));
        ProfileVault legacyOnly(encrypted->path(), working->path(), KEY_A);
        newOnly.clearWorkingDirectory();
        QVERIFY(!legacyOnly.restore("one"));
    }

    void whileSavingIsOffNoSavedFileIsDeleted()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "x");
        QVERIFY(vault.seal("one"));
        vault.clearWorkingDirectory();

        ProfileVault withoutKey(encrypted->path(), working->path(), {});
        QVERIFY(!withoutKey.restore("one"));
        QVERIFY(QFile::exists(encrypted->path() + "/one.vault"));
    }

    void cachesOfTheEngineAreNotSaved()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "keep");
        writeText(vault.storagePath("one") + "/GPUCache/data_0", "drop");
        QVERIFY(vault.seal("one"));
        vault.clearWorkingDirectory();
        QVERIFY(vault.restore("one"));
        QVERIFY(QFile::exists(vault.storagePath("one") + "/Cookies"));
        QVERIFY(!QFile::exists(vault.storagePath("one") + "/GPUCache/data_0"));
    }

    void discardRemovesTheFileAndTheWorkingFolder()
    {
        ProfileVault vault(encrypted->path(), working->path(), KEY_A);
        vault.restore("one");
        writeText(vault.storagePath("one") + "/Cookies", "x");
        QVERIFY(vault.seal("one"));
        vault.discard("one");
        QVERIFY(!QFile::exists(encrypted->path() + "/one.vault"));
        QVERIFY(!QDir(vault.storagePath("one")).exists());
    }

    void archiveWithAPathOutsideTheFolderIsRejected()
    {
        QByteArray archive;
        QDataStream stream(&archive, QIODevice::WriteOnly);
        stream << quint32(1) << QString("../escape.txt") << QByteArray("x");
        QVERIFY(!unpackInto(qCompress(archive), QDir(working->path())));
        QVERIFY(!QFile::exists(QFileInfo(working->path()).absolutePath() + "/escape.txt"));
    }

private:
    std::unique_ptr<QTemporaryDir> encrypted;
    std::unique_ptr<QTemporaryDir> working;
};

QTEST_APPLESS_MAIN(TestProfileVault)
#include "test_profile_vault.moc"
