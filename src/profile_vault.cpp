// SPDX-License-Identifier: AGPL-3.0-or-later
#include "profile_vault.h"

#include "crypto.h"

#include <QDataStream>
#include <QDirIterator>
#include <QFile>
#include <QSaveFile>
#include <QStringList>

namespace portails {

namespace {

constexpr quint32 ARCHIVE_VERSION = 1;
const QStringList TRANSIENT_DIRECTORIES = {"GPUCache", "Code Cache", "DawnGraphiteCache", "DawnWebGPUCache",
                                           "Crashpad"};

bool isTransient(const QString &relativePath)
{
    for (const QString &name : TRANSIENT_DIRECTORIES) {
        if (relativePath == name || relativePath.startsWith(name + '/')) {
            return true;
        }
    }
    return false;
}

bool isSafeRelativePath(const QString &relativePath)
{
    if (relativePath.isEmpty() || QDir::isAbsolutePath(relativePath)) {
        return false;
    }
    return !relativePath.split('/').contains("..");
}

bool writeFile(const QString &path, const QByteArray &content)
{
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
    file.write(content);
    return file.commit();
}

bool makePrivateDirectory(const QString &path)
{
    if (!QDir().mkpath(path)) {
        return false;
    }
    return QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
}

QByteArray tabContext(const QString &tabId)
{
    return "tab:" + tabId.toUtf8();
}

}  // namespace

QByteArray readMachineId()
{
    for (const QString &path : {QStringLiteral("/etc/machine-id"), QStringLiteral("/var/lib/dbus/machine-id")}) {
        QFile file(path);
        if (file.open(QIODevice::ReadOnly)) {
            return file.readAll().trimmed();
        }
    }
    return {};
}

QByteArray packDirectory(const QDir &directory)
{
    QByteArray archive;
    QDataStream stream(&archive, QIODevice::WriteOnly);
    stream << ARCHIVE_VERSION;
    QDirIterator iterator(directory.absolutePath(), QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
        const QString relativePath = directory.relativeFilePath(iterator.next());
        QFile file(iterator.filePath());
        if (isTransient(relativePath) || !file.open(QIODevice::ReadOnly)) {
            continue;
        }
        stream << relativePath << file.readAll();
    }
    return qCompress(archive, 6);
}

bool unpackInto(const QByteArray &compressed, const QDir &directory)
{
    QByteArray archive = qUncompress(compressed);
    QDataStream stream(&archive, QIODevice::ReadOnly);
    quint32 version = 0;
    stream >> version;
    if (version != ARCHIVE_VERSION) {
        return false;
    }
    while (!stream.atEnd()) {
        QString relativePath;
        QByteArray content;
        stream >> relativePath >> content;
        if (stream.status() != QDataStream::Ok || !isSafeRelativePath(relativePath)) {
            return false;
        }
        const QString target = directory.absoluteFilePath(relativePath);
        if (!QDir().mkpath(QFileInfo(target).absolutePath()) || !writeFile(target, content)) {
            return false;
        }
    }
    return true;
}

ProfileVault::ProfileVault(const QString &encryptedDirectory, const QString &workingDirectory, const QByteArray &key,
                           const QByteArray &legacyKey)
    : encryptedDirectory(encryptedDirectory)
    , workingDirectory(workingDirectory)
    , key(key)
    , legacyKey(legacyKey)
{
    makePrivateDirectory(encryptedDirectory);
    makePrivateDirectory(workingDirectory);
}

QString ProfileVault::storagePath(const QString &tabId) const
{
    return workingDirectory + "/" + tabId + "/storage";
}

QString ProfileVault::cachePath(const QString &tabId) const
{
    return workingDirectory + "/" + tabId + "/cache";
}

QString ProfileVault::blobPath(const QString &tabId) const
{
    return encryptedDirectory + "/" + tabId + ".vault";
}

std::optional<QByteArray> ProfileVault::decrypt(const QByteArray &blob, const QString &tabId) const
{
    for (const QByteArray &candidate : {key, legacyKey}) {
        if (candidate.isEmpty()) {
            continue;
        }
        if (const std::optional<QByteArray> plain = decryptBlob(candidate, blob, tabContext(tabId))) {
            return plain;
        }
    }
    return std::nullopt;
}

bool ProfileVault::restore(const QString &tabId)
{
    makePrivateDirectory(storagePath(tabId));
    QFile file(blobPath(tabId));
    if (!isSaving() || !file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const std::optional<QByteArray> plain = decrypt(file.readAll(), tabId);
    file.close();
    if (plain && unpackInto(*plain, QDir(storagePath(tabId)))) {
        return true;
    }
    discard(tabId);
    makePrivateDirectory(storagePath(tabId));
    return false;
}

bool ProfileVault::seal(const QString &tabId) const
{
    if (!isSaving() || !QDir(storagePath(tabId)).exists()) {
        return false;
    }
    const QByteArray blob = encryptBlob(key, packDirectory(QDir(storagePath(tabId))), tabContext(tabId));
    return !blob.isEmpty() && writeFile(blobPath(tabId), blob);
}

void ProfileVault::discard(const QString &tabId) const
{
    QFile::remove(blobPath(tabId));
    QDir(workingDirectory + "/" + tabId).removeRecursively();
}

void ProfileVault::clearWorkingDirectory() const
{
    QDir directory(workingDirectory);
    for (const QString &name : directory.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QDir(directory.absoluteFilePath(name)).removeRecursively();
    }
}

}  // namespace portails
