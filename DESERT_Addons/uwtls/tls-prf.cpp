#include "tls-prf.h"

namespace key_derivation {

/**
 * Derives TLS 1.2 PRF output using OpenSSL EVP_PKEY_TLS1_PRF.
 *
 * @param out Output vector that will contain the derived bytes.
 * @param secret Pointer to the secret input.
 * @param secret_len Secret length in bytes.
 * @param label Pointer to the ASCII label bytes.
 * @param label_len Label length in bytes, without terminating null.
 * @param seed1 Pointer to the first seed fragment.
 * @param seed1_len Length of the first seed fragment.
 * @param seed2 Pointer to the second seed fragment.
 * @param seed2_len Length of the second seed fragment.
 * @param out_len Number of bytes to derive.
 *
 * @returns Result of the OpenSSL PRF derivation.
 */
static DerivationResult derive_tls12_prf(std::vector<uint8_t>& out,
                                         const uint8_t* secret,
                                         std::size_t secret_len,
                                         const uint8_t* label,
                                         std::size_t label_len,
                                         const uint8_t* seed1,
                                         std::size_t seed1_len,
                                         const uint8_t* seed2,
                                         std::size_t seed2_len,
                                         std::size_t out_len)
{
	if (secret == nullptr || secret_len == 0) {
		// Invalid PRF secret
		return DerivationResult::PRF_ERROR;
	}
	if (label == nullptr || label_len == 0) {
		// Invalid PRF label
		return DerivationResult::PRF_ERROR;
	}
	if (seed1 == nullptr || seed1_len == 0 || seed2 == nullptr || seed2_len == 0) {
		// Invalid PRF seed
		return DerivationResult::PRF_ERROR;
	}
	if (out_len == 0) {
		// Invalid PRF output length
		return DerivationResult::PRF_ERROR;
	}

	EVP_PKEY_CTX* pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_TLS1_PRF, nullptr);
	if (pctx == nullptr) {
		// EVP_PKEY_CTX_new_id failed
		return DerivationResult::PRF_ERROR;
	}

	try {
		if (EVP_PKEY_derive_init(pctx) <= 0) {
			// EVP_PKEY_derive_init failed
			return DerivationResult::PRF_ERROR;
		}

		if (EVP_PKEY_CTX_set_tls1_prf_md(pctx, EVP_sha256()) <= 0) {
			// EVP_PKEY_CTX_set_tls1_prf_md failed
			return DerivationResult::PRF_ERROR;
		}

		if (EVP_PKEY_CTX_set1_tls1_prf_secret(
				pctx,
				secret,
				static_cast<int>(secret_len)) <= 0) {
			// EVP_PKEY_CTX_set1_tls1_prf_secret failed
			return DerivationResult::PRF_ERROR;
		}

		if (EVP_PKEY_CTX_add1_tls1_prf_seed(
				pctx,
				label,
				static_cast<int>(label_len)) <= 0) {
			// EVP_PKEY_CTX_add1_tls1_prf_seed label failed
			return DerivationResult::PRF_ERROR;
		}

		if (EVP_PKEY_CTX_add1_tls1_prf_seed(
				pctx,
				seed1,
				static_cast<int>(seed1_len)) <= 0) {
			// EVP_PKEY_CTX_add1_tls1_prf_seed seed1 failed
			return DerivationResult::PRF_ERROR;
		}

		if (EVP_PKEY_CTX_add1_tls1_prf_seed(
				pctx,
				seed2,
				static_cast<int>(seed2_len)) <= 0) {
			// EVP_PKEY_CTX_add1_tls1_prf_seed seed2 failed
			return DerivationResult::PRF_ERROR;
		}

		out.resize(out_len);
		std::size_t derived_len = out_len;

		if (EVP_PKEY_derive(pctx, out.data(), &derived_len) <= 0) {
			// EVP_PKEY_derive failed
			return DerivationResult::PRF_ERROR;
		}

		if (derived_len != out_len) {
			// Unexpected derived length
			return DerivationResult::PRF_ERROR;
		}
	} catch (...) {
        EVP_PKEY_CTX_free(pctx);
        return DerivationResult::PRF_ERROR;
	}

	EVP_PKEY_CTX_free(pctx);

    return DerivationResult::OK;
}

DerivationResult derive_tls12_key_block(std::vector<uint8_t>& key_block,
                                        const uint8_t premaster[48],
                                        std::size_t premaster_len,
                                        const uint8_t client_random[32],
                                        const uint8_t server_random[32],
                                        std::size_t out_len)
{
	static const char master_label[] = "master secret";
	static const char keyexp_label[] = "key expansion";

	if (premaster == nullptr) {
		return DerivationResult::PREMASTER_ERROR;
	}
	if (client_random == nullptr || server_random == nullptr) {
		return DerivationResult::RANDOMS_ERROR;
	}

	DerivationResult result;
	std::vector<uint8_t> master_secret;

	result = derive_tls12_prf(master_secret,
	                          premaster,
	                          premaster_len,
	                          reinterpret_cast<const uint8_t*>(master_label),
	                          sizeof(master_label) - 1,
	                          client_random,
	                          32,
	                          server_random,
	                          32,
	                          48);

	if (result != DerivationResult::OK) {
		return result;
	}

	result = derive_tls12_prf(key_block,
	                          master_secret.data(),
	                          master_secret.size(),
	                          reinterpret_cast<const uint8_t*>(keyexp_label),
	                          sizeof(keyexp_label) - 1,
	                          server_random,
	                          32,
	                          client_random,
	                          32,
	                          out_len);

	return result;
}

} // namespace key_derivation
