/*
 * Copyright (C) 2023 YTX
 *
 * This file is part of YTX.
 *
 * YTX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * YTX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with YTX. If not, see <https://www.gnu.org/licenses/>.
 */

#include <QByteArray>
#include <QString>

#ifndef PASSWORD_ENCRYPTION_H
#define PASSWORD_ENCRYPTION_H

// Encrypted format:
// Base64( nonce[12] | ciphertext[N] | tag[16] )

class PasswordEncryption {
public:
    static QString Encrypt(const QString& plaintext, const QByteArray& machine_key);
    static QString Decrypt(const QString& ciphertext, const QByteArray& machine_key);
    static QByteArray GetMachineKey();

private:
    static constexpr int KEY_SIZE = 32; // ChaCha20-Poly1305 key size
    static constexpr int NONCE_SIZE = 12; // ChaCha20-Poly1305 nonce size
    static constexpr int TAG_SIZE = 16; // ChaCha20-Poly1305 tag size
};

#endif // PASSWORD_ENCRYPTION_H
