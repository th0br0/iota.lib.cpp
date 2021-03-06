//
// MIT License
//
// Copyright (c) 2017-2018 Thibault Martinez and Simon Ninon
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
//

#include <algorithm>

#include <iota/constants.hpp>
#include <iota/crypto/kerl.hpp>
#include <iota/crypto/signing.hpp>
#include <iota/models/bundle.hpp>
#include <iota/models/seed.hpp>
#include <iota/types/big_int.hpp>
#include <iota/types/trinary.hpp>

namespace IOTA {

namespace Crypto {

namespace Signing {

//! When a tryte value is normalized, it is converted into a list of integers.
//! The int values ranged from -13 to 13 (giving a set of 27 values, matching the alphabet length)
//! This characteristic is used in the Signing algorithm
static constexpr int NormalizedTryteUpperBound = 13;

std::vector<uint8_t>
key(const std::vector<uint8_t>& seedBytes, uint32_t index, uint32_t security) {
  Kerl k;

  Types::Bigint        b;
  std::vector<uint8_t> seedIndexBytes(ByteHashLength);

  b.fromBytes(seedBytes);
  b.addU32(index);
  b.toBytes(seedIndexBytes);

  k.absorb(seedIndexBytes);
  k.finalSqueeze(seedIndexBytes);
  k.reset();
  k.absorb(seedIndexBytes);

  std::vector<uint8_t> keyBytes(security * FragmentLength * ByteHashLength);

  unsigned int offset = 0;
  for (unsigned int i = 0; i < security; ++i) {
    for (unsigned int j = 0; j < FragmentLength; ++j) {
      k.squeeze(keyBytes, offset);
      offset += ByteHashLength;
    }
  }
  return keyBytes;
}

std::vector<uint8_t>
digests(const std::vector<uint8_t>& key) {
  Kerl                 k1;
  Kerl                 k2;
  unsigned int         security = key.size() / (ByteHashLength * FragmentLength);
  std::vector<uint8_t> digests(security * ByteHashLength);

  for (unsigned int i = 0; i < security; ++i) {
    std::vector<uint8_t> keyFragment(key.begin() + (i * ByteHashLength * FragmentLength),
                                     key.begin() + ((i + 1) * ByteHashLength * FragmentLength));
    for (unsigned j = 0; j < FragmentLength; ++j) {
      auto fragmentOffset = j * ByteHashLength;
      for (unsigned int l = 0; l < FragmentLength - 1; ++l) {
        k1.reset();
        k1.absorb(keyFragment, fragmentOffset, ByteHashLength);
        k1.finalSqueeze(keyFragment, fragmentOffset);
      }
      k2.absorb(keyFragment, fragmentOffset, ByteHashLength);
    }
    k2.finalSqueeze(digests, i * ByteHashLength);
    k2.reset();
  }
  return digests;
}

std::vector<uint8_t>
address(const std::vector<uint8_t>& digests) {
  Kerl                 k;
  std::vector<uint8_t> addressBytes(ByteHashLength);

  k.absorb(digests);
  k.finalSqueeze(addressBytes);
  return addressBytes;
}

std::vector<uint8_t>
digest(const std::vector<int8_t>&  normalizedBundleFragment,
       const std::vector<uint8_t>& signatureFragment) {
  Kerl k1;
  Kerl k2;

  for (unsigned int i = 0; i < FragmentLength; i++) {
    std::vector<uint8_t> buffer(&signatureFragment[i * ByteHashLength],
                                &signatureFragment[(i + 1) * ByteHashLength]);
    for (unsigned int j = normalizedBundleFragment[i] + NormalizedTryteUpperBound; j > 0; --j) {
      k2.reset();
      k2.absorb(buffer);
      k2.finalSqueeze(buffer);
    }
    k1.absorb(buffer);
  }
  std::vector<uint8_t> buffer(ByteHashLength);
  k1.finalSqueeze(buffer);
  return buffer;
}

// TODO(Optimize)
Types::Trits
signatureFragment(const std::vector<int8_t>& normalizedBundleFragment,
                  const Types::Trits&        keyFragment) {
  Kerl         k;
  Types::Trits signatureFragment;

  for (unsigned int i = 0; i < FragmentLength; ++i) {
    Types::Trits buffer(keyFragment.begin() + i * TritHashLength,
                        keyFragment.begin() + (i + 1) * TritHashLength);
    auto         bytes = Types::tritsToBytes(buffer);
    for (int j = 0; j < NormalizedTryteUpperBound - normalizedBundleFragment[i]; ++j) {
      k.reset();
      k.absorb(bytes);
      k.finalSqueeze(bytes);
    }
    auto buff = Types::bytesToTrits(bytes);
    signatureFragment.insert(std::end(signatureFragment), std::begin(buff), std::end(buff));
  }

  return signatureFragment;
}

std::vector<Types::Trytes>
signInputs(const Models::Seed& seed, const std::vector<Models::Address>& inputs,
           Models::Bundle& bundle, const std::vector<Types::Trytes>& signatureFragments) {
  bundle.finalize();
  bundle.addTrytes(signatureFragments);

  //  SIGNING OF INPUTS
  //
  //  Here we do the actual signing of the inputs
  //  Iterate over all bundle transactions, find the inputs
  //  Get the corresponding private key and calculate the signatureFragment
  for (auto& tx : bundle.getTransactions()) {
    if (tx.getValue() < 0) {
      auto addr = tx.getAddress();

      // Get the corresponding keyIndex of the address
      int keyIndex    = 0;
      int keySecurity = 0;
      for (const auto& input : inputs) {
        if (input == addr) {
          keyIndex    = input.getKeyIndex();
          keySecurity = input.getSecurity();
        }
      }

      auto bundleHash = tx.getBundle();

      // Get corresponding private key of address
      auto key = Types::bytesToTrits(Crypto::Signing::key(
          Types::trytesToBytes(seed.toTrytes()), keyIndex, seed.getSecurity()));  // TODO(optimize)

      //  First 6561 trits for the firstFragment
      std::vector<int8_t> firstFragment(&key[0], &key[6561]);

      //  Get the normalized bundle hash
      auto normalizedBundleHash = bundle.normalizedBundle(bundleHash);

      //  First bundle fragment uses 27 trytes
      std::vector<int8_t> firstBundleFragment(&normalizedBundleHash[0], &normalizedBundleHash[27]);

      //  Calculate the new signatureFragment with the first bundle fragment
      auto firstSignedFragment =
          Crypto::Signing::signatureFragment(firstBundleFragment, firstFragment);

      //  Convert signature to trytes and assign the new signatureFragment
      tx.setSignatureFragments(Types::tritsToTrytes(firstSignedFragment));

      // if user chooses higher than 27-tryte security
      // for each security level, add an additional signature
      for (int j = 1; j < keySecurity; j++) {
        //  Because the signature is > 2187 trytes, we need to
        //  find the second transaction to add the remainder of the signature
        for (auto& txb : bundle.getTransactions()) {
          //  Same address as well as value = 0 (as we already spent the input)
          if (txb.getAddress() == addr && txb.getValue() == 0) {
            // Use the second 6562 trits
            std::vector<int8_t> secondFragment(&key[6561], &key[6561 * 2]);

            // The second 27 to 54 trytes of the bundle hash
            std::vector<int8_t> secondBundleFragment(&normalizedBundleHash[27],
                                                     &normalizedBundleHash[27 * 2]);

            //  Calculate the new signature
            auto secondSignedFragment =
                Crypto::Signing::signatureFragment(secondBundleFragment, secondFragment);

            //  Convert signature to trytes and assign it again to this bundle entry
            txb.setSignatureFragments(Types::tritsToTrytes(secondSignedFragment));
          }
        }
      }
    }
  }

  std::vector<Types::Trytes> bundleTrytes;

  std::sort(bundle.getTransactions().begin(), bundle.getTransactions().end(),
            [](const Models::Transaction& lhs, const Models::Transaction& rhs) {
              return lhs.getCurrentIndex() < rhs.getCurrentIndex();
            });

  // Convert all bundle entries into trytes
  for (const auto& tx : bundle.getTransactions()) {
    bundleTrytes.emplace_back(tx.toTrytes());
  }

  return bundleTrytes;
}

bool
validateSignatures(const Models::Address&            expectedAddress,
                   const std::vector<Types::Trytes>& signatureFragments,
                   const Types::Trytes&              bundleHash) {
  Models::Bundle                   bundle;
  auto                             normalizedBundleHash = bundle.normalizedBundle(bundleHash);
  std::vector<std::vector<int8_t>> normalizedBundleFragments;
  std::vector<uint8_t>             digests;

  for (unsigned int i = 0; i < 3; i++) {
    normalizedBundleFragments.emplace_back(normalizedBundleHash.begin() + i * FragmentLength,
                                           normalizedBundleHash.begin() + (i + 1) * FragmentLength);
  }

  for (unsigned int i = 0; i < signatureFragments.size(); ++i) {
    auto digestBuffer =
        digest(normalizedBundleFragments[i % 3], Types::trytesToBytes(signatureFragments[i]));

    digests.insert(std::end(digests), std::begin(digestBuffer), std::end(digestBuffer));
  }

  return expectedAddress == Types::bytesToTrytes(address(digests));
}

}  // namespace Signing

}  // namespace Crypto

}  // namespace IOTA
