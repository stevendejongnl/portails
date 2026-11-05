// SPDX-License-Identifier: AGPL-3.0-or-later
#pragma once

#include <QByteArray>
#include <QDir>
#include <QString>

#include <optional>

namespace portails {

QByteArray readMachineId();

// Keeps the plain profile of each tab in a RAM-backed folder while the app runs.
// On disk there is only one encrypted file per tab.
class ProfileVault {
public:
    // An empty key turns saving off: profiles then live in RAM only and are gone after exit.
    // Saved files are never deleted while saving is off.
    // The legacy key only opens files that were saved with an older key. The next seal uses the new key.
    ProfileVault(const QString &encryptedDirectory, const QString &workingDirectory, const QByteArray &key,
                 const QByteArray &legacyKey = {});

    bool isSaving() const { return !key.isEmpty(); }

    QString storagePath(const QString &tabId) const;
    QString cachePath(const QString &tabId) const;

    // Decrypts the saved profile of a tab into its working folder.
    // When that fails, the saved file is deleted and the tab starts logged out.
    bool restore(const QString &tabId);

    // Encrypts the working folder of a tab and replaces the saved file.
    bool seal(const QString &tabId) const;

    // Deletes the saved file and the working folders of a tab.
    void discard(const QString &tabId) const;

    void clearWorkingDirectory() const;

private:
    QString blobPath(const QString &tabId) const;
    std::optional<QByteArray> decrypt(const QByteArray &blob, const QString &tabId) const;

    QString encryptedDirectory;
    QString workingDirectory;
    QByteArray key;
    QByteArray legacyKey;
};

// Older builds left a QtWebEngine folder in the data folder. It held only empty folders.
// It is removed when it has no file in it. A folder with files in it is never touched.
bool removeEmptyEngineFolder(const QString &dataDirectory);

// Archive format used inside the encrypted file. Exposed for tests.
QByteArray packDirectory(const QDir &directory);
bool unpackInto(const QByteArray &archive, const QDir &directory);

}  // namespace portails
