/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef JSCryptoAlgorithmDictionary_h
#define JSCryptoAlgorithmDictionary_h

#include "CryptoAlgorithmIdentifier.h"
#include <wtf/RefPtr.h>

#if ENABLE(SUBTLE_CRYPTO)

namespace JSC {
class ExecState;
class JSValue;
}

namespace WebCore {

class CryptoAlgorithmParameters;

class JSCryptoAlgorithmDictionary {
public:
    static bool getAlgorithmIdentifier(JSC::ExecState*, JSC::JSValue, CryptoAlgorithmIdentifier&);

    static RefPtr<CryptoAlgorithmParameters> createParametersForEncrypt(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForDecrypt(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForSign(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForVerify(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForDigest(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForGenerateKey(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForDeriveKey(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForDeriveBits(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForImportKey(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
    static RefPtr<CryptoAlgorithmParameters> createParametersForExportKey(JSC::ExecState*, CryptoAlgorithmIdentifier, JSC::JSValue);
};

}

#endif // ENABLE(SUBTLE_CRYPTO)
#endif // JSCryptoAlgorithmDictionary_h
