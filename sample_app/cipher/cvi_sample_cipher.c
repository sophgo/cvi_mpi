/*
 * Copyright (C) Cvitek Co., Ltd. 2019-2020. All rights reserved.
 *
 * File Name: cvi_sample_cipher.c
 * Description: Cipher encrypt/decrypt/hash/random/multi-package test
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <cvi_sample_all.h>
#include <cvi_unf_cipher.h>

#define CIPHER_TEST_DATA_LEN    64
#define CIPHER_MAX_OUTPUT_LEN   128

#define CHECK_RESULT(ret, ...)                                                                                         \
	do {                                                                                                           \
		if (ret != CVI_SUCCESS) {                                                                              \
			CVI_ERR_CIPHER(__VA_ARGS__);                                                                   \
			return CVI_FAILURE;                                                                            \
		}                                                                                                      \
	} while (0)

// Test data
static CVI_U8 test_plain_data[CIPHER_TEST_DATA_LEN] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
	0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
	0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
};

// AES-128 key
static CVI_U8 aes_key_128[16] = {
	0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C
};

// AES-256 key
static CVI_U8 aes_key_256[32] = {
	0x60, 0x3D, 0xEB, 0x10, 0x15, 0xCA, 0x71, 0xBE, 0x2B, 0x73, 0xAE, 0xF0, 0x85, 0x7D, 0x77, 0x81,
	0x1F, 0x35, 0x2C, 0x07, 0x3B, 0x61, 0x08, 0xD7, 0x2D, 0x98, 0x10, 0xA3, 0x09, 0x14, 0xDF, 0xF4
};

// Initialization vector
static CVI_U8 aes_iv[16] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F
};

// Test AES-128 CBC encryption and decryption
static CVI_S32 test_aes_128_cbc(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_CIPHER_HANDLE hCipher;
	CVI_UNF_CIPHER_ATTS_S stCipherAttr;
	CVI_UNF_CIPHER_CTRL_S stCipherCtrl;
	CVI_U8 encrypted_data[CIPHER_MAX_OUTPUT_LEN];
	CVI_U8 decrypted_data[CIPHER_MAX_OUTPUT_LEN];
	
	CVI_INFO_CIPHER("=== Testing AES-128 CBC ===\n");
	
	// Set Cipher attributes
	memset(&stCipherAttr, 0, sizeof(stCipherAttr));
	stCipherAttr.enCipherType = CVI_UNF_CIPHER_TYPE_NORMAL;
	
	// Create Cipher handle
	ret = CVI_UNF_CIPHER_CreateHandle(&hCipher, &stCipherAttr);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_CreateHandle failed, ret=0x%x\n", ret);
	
	// Configure Cipher control parameters
	memset(&stCipherCtrl, 0, sizeof(stCipherCtrl));
	stCipherCtrl.enAlg = CVI_UNF_CIPHER_ALG_AES;
	stCipherCtrl.enWorkMode = CVI_UNF_CIPHER_WORK_MODE_CBC;
	stCipherCtrl.enKeyLen = CVI_UNF_CIPHER_KEY_AES_128BIT;
	memcpy(stCipherCtrl.u32Key, aes_key_128, sizeof(aes_key_128));
	memcpy(stCipherCtrl.u32IV, aes_iv, sizeof(aes_iv));
	
	ret = CVI_UNF_CIPHER_ConfigHandle(hCipher, &stCipherCtrl);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_ConfigHandle failed, ret=0x%x\n", ret);
	
	// Encrypt
	memset(encrypted_data, 0, sizeof(encrypted_data));
	ret = CVI_UNF_CIPHER_EncryptVir(hCipher, test_plain_data, encrypted_data, CIPHER_TEST_DATA_LEN);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_EncryptVir failed, ret=0x%x\n", ret);
	
	CVI_INFO_CIPHER("AES-128 CBC encryption completed\n");
	CVI_Test_PrintBuffer("Encrypted data", encrypted_data, CIPHER_TEST_DATA_LEN);
	
	// Decrypt
	memset(decrypted_data, 0, sizeof(decrypted_data));
	ret = CVI_UNF_CIPHER_DecryptVir(hCipher, encrypted_data, decrypted_data, CIPHER_TEST_DATA_LEN);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_DecryptVir failed, ret=0x%x\n", ret);
	
	CVI_INFO_CIPHER("AES-128 CBC decryption completed\n");
	CVI_Test_PrintBuffer("Decrypted data", decrypted_data, CIPHER_TEST_DATA_LEN);
	
	// Verify decryption result
	if (memcmp(test_plain_data, decrypted_data, CIPHER_TEST_DATA_LEN) == 0) {
		CVI_INFO_CIPHER("AES-128 CBC test PASSED\n");
	} else {
		CVI_ERR_CIPHER("AES-128 CBC test FAILED - data mismatch\n");
		ret = CVI_FAILURE;
	}
	
	// Destroy handle
	CVI_UNF_CIPHER_DestroyHandle(hCipher);
	
	return ret;
}

// Test AES-256 ECB encryption and decryption
static CVI_S32 test_aes_256_ecb(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_CIPHER_HANDLE hCipher;
	CVI_UNF_CIPHER_ATTS_S stCipherAttr;
	CVI_UNF_CIPHER_CTRL_S stCipherCtrl;
	CVI_U8 encrypted_data[CIPHER_MAX_OUTPUT_LEN];
	CVI_U8 decrypted_data[CIPHER_MAX_OUTPUT_LEN];
	
	CVI_INFO_CIPHER("=== Testing AES-256 ECB ===\n");
	
	// Set Cipher attributes
	memset(&stCipherAttr, 0, sizeof(stCipherAttr));
	stCipherAttr.enCipherType = CVI_UNF_CIPHER_TYPE_NORMAL;
	
	// Create Cipher handle
	ret = CVI_UNF_CIPHER_CreateHandle(&hCipher, &stCipherAttr);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_CreateHandle failed, ret=0x%x\n", ret);
	
	// Configure Cipher control parameters
	memset(&stCipherCtrl, 0, sizeof(stCipherCtrl));
	stCipherCtrl.enAlg = CVI_UNF_CIPHER_ALG_AES;
	stCipherCtrl.enWorkMode = CVI_UNF_CIPHER_WORK_MODE_ECB;
	stCipherCtrl.enKeyLen = CVI_UNF_CIPHER_KEY_AES_256BIT;
	memcpy(stCipherCtrl.u32Key, aes_key_256, sizeof(aes_key_256));
	
	ret = CVI_UNF_CIPHER_ConfigHandle(hCipher, &stCipherCtrl);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_ConfigHandle failed, ret=0x%x\n", ret);
	
	// Encrypt
	memset(encrypted_data, 0, sizeof(encrypted_data));
	ret = CVI_UNF_CIPHER_EncryptVir(hCipher, test_plain_data, encrypted_data, CIPHER_TEST_DATA_LEN);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_EncryptVir failed, ret=0x%x\n", ret);
	
	CVI_INFO_CIPHER("AES-256 ECB encryption completed\n");
	CVI_Test_PrintBuffer("Encrypted data", encrypted_data, CIPHER_TEST_DATA_LEN);
	
	// Decrypt
	memset(decrypted_data, 0, sizeof(decrypted_data));
	ret = CVI_UNF_CIPHER_DecryptVir(hCipher, encrypted_data, decrypted_data, CIPHER_TEST_DATA_LEN);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_DecryptVir failed, ret=0x%x\n", ret);
	
	CVI_INFO_CIPHER("AES-256 ECB decryption completed\n");
	CVI_Test_PrintBuffer("Decrypted data", decrypted_data, CIPHER_TEST_DATA_LEN);
	
	// Verify decryption result
	if (memcmp(test_plain_data, decrypted_data, CIPHER_TEST_DATA_LEN) == 0) {
		CVI_INFO_CIPHER("AES-256 ECB test PASSED\n");
	} else {
		CVI_ERR_CIPHER("AES-256 ECB test FAILED - data mismatch\n");
		ret = CVI_FAILURE;
	}
	
	// Destroy handle
	CVI_UNF_CIPHER_DestroyHandle(hCipher);
	
	return ret;
}

// Test hash functionality
static CVI_S32 test_hash_sha256(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_CIPHER_HANDLE hHashHandle;
	CVI_UNF_CIPHER_HASH_ATTS_S stHashAttr;
	CVI_U8 hash_output[32]; // SHA256 output 32 bytes
	
	CVI_INFO_CIPHER("=== Testing SHA256 Hash ===\n");
	
	// Set hash attributes
	memset(&stHashAttr, 0, sizeof(stHashAttr));
	stHashAttr.eShaType = CVI_UNF_CIPHER_HASH_TYPE_SHA256;
	
	// Initialize hash
	ret = CVI_UNF_CIPHER_HashInit(&stHashAttr, &hHashHandle);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_HashInit failed, ret=0x%x\n", ret);
	
	// Update hash data
	ret = CVI_UNF_CIPHER_HashUpdate(hHashHandle, test_plain_data, CIPHER_TEST_DATA_LEN);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_HashUpdate failed, ret=0x%x\n", ret);
	
	// Complete hash calculation
	memset(hash_output, 0, sizeof(hash_output));
	ret = CVI_UNF_CIPHER_HashFinal(hHashHandle, hash_output);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_HashFinal failed, ret=0x%x\n", ret);
	
	CVI_INFO_CIPHER("SHA256 hash calculation completed\n");
	CVI_Test_PrintBuffer("SHA256 hash", hash_output, 32);
	
	return ret;
}

// Test random number generation
static CVI_S32 test_random_number(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_U32 random_numbers[8];
	int i;
	
	CVI_INFO_CIPHER("=== Testing Random Number Generation ===\n");
	
	// Generate multiple random numbers
	for (i = 0; i < 8; i++) {
		ret = CVI_UNF_CIPHER_GetRandomNumber(&random_numbers[i]);
		CHECK_RESULT(ret, "CVI_UNF_CIPHER_GetRandomNumber failed, ret=0x%x\n", ret);
	}
	
	CVI_INFO_CIPHER("Generated random numbers:\n");
	for (i = 0; i < 8; i++) {
		CVI_INFO_CIPHER("Random[%d]: 0x%08x\n", i, random_numbers[i]);
	}
	
	return ret;
}

// Test multi-package encryption and decryption
static CVI_S32 test_multi_package_encrypt(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	CVI_CIPHER_HANDLE hCipher;
	CVI_UNF_CIPHER_ATTS_S stCipherAttr;
	CVI_UNF_CIPHER_CTRL_S stCipherCtrl;
	CVI_UNF_CIPHER_DATA_S stDataPkg[2];
	CVI_U8 encrypted_data1[32], encrypted_data2[32];
	CVI_U8 decrypted_data1[32], decrypted_data2[32];
	
	CVI_INFO_CIPHER("=== Testing Multi-Package Encryption ===\n");
	
	// Set Cipher attributes
	memset(&stCipherAttr, 0, sizeof(stCipherAttr));
	stCipherAttr.enCipherType = CVI_UNF_CIPHER_TYPE_NORMAL;
	
	// Create Cipher handle
	ret = CVI_UNF_CIPHER_CreateHandle(&hCipher, &stCipherAttr);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_CreateHandle failed, ret=0x%x\n", ret);
	
	// Configure Cipher control parameters
	memset(&stCipherCtrl, 0, sizeof(stCipherCtrl));
	stCipherCtrl.enAlg = CVI_UNF_CIPHER_ALG_AES;
	stCipherCtrl.enWorkMode = CVI_UNF_CIPHER_WORK_MODE_ECB;
	stCipherCtrl.enKeyLen = CVI_UNF_CIPHER_KEY_AES_128BIT;
	memcpy(stCipherCtrl.u32Key, aes_key_128, sizeof(aes_key_128));
	
	ret = CVI_UNF_CIPHER_ConfigHandle(hCipher, &stCipherCtrl);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_ConfigHandle failed, ret=0x%x\n", ret);
	
	// Prepare data packages
	memset(stDataPkg, 0, sizeof(stDataPkg));
	
	// First data package
	stDataPkg[0].szSrcAddr = (CVI_SIZE_T)test_plain_data;
	stDataPkg[0].szDestAddr = (CVI_SIZE_T)encrypted_data1;
	stDataPkg[0].u32ByteLength = 32;
	stDataPkg[0].bOddKey = CVI_FALSE;
	
	// Second data package
	stDataPkg[1].szSrcAddr = (CVI_SIZE_T)(test_plain_data + 32);
	stDataPkg[1].szDestAddr = (CVI_SIZE_T)encrypted_data2;
	stDataPkg[1].u32ByteLength = 32;
	stDataPkg[1].bOddKey = CVI_FALSE;
	
	// Multi-package encryption
	ret = CVI_UNF_CIPHER_EncryptMulti(hCipher, stDataPkg, 2);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_EncryptMulti failed, ret=0x%x\n", ret);
	
	CVI_INFO_CIPHER("Multi-package encryption completed\n");
	CVI_Test_PrintBuffer("Encrypted package 1", encrypted_data1, 32);
	CVI_Test_PrintBuffer("Encrypted package 2", encrypted_data2, 32);
	
	// Prepare decryption data packages
	stDataPkg[0].szSrcAddr = (CVI_SIZE_T)encrypted_data1;
	stDataPkg[0].szDestAddr = (CVI_SIZE_T)decrypted_data1;
	stDataPkg[1].szSrcAddr = (CVI_SIZE_T)encrypted_data2;
	stDataPkg[1].szDestAddr = (CVI_SIZE_T)decrypted_data2;
	
	// Multi-package decryption
	ret = CVI_UNF_CIPHER_DecryptMulti(hCipher, stDataPkg, 2);
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_DecryptMulti failed, ret=0x%x\n", ret);
	
	CVI_INFO_CIPHER("Multi-package decryption completed\n");
	CVI_Test_PrintBuffer("Decrypted package 1", decrypted_data1, 32);
	CVI_Test_PrintBuffer("Decrypted package 2", decrypted_data2, 32);
	
	// Verify decryption result
	if (memcmp(test_plain_data, decrypted_data1, 32) == 0 && 
	    memcmp(test_plain_data + 32, decrypted_data2, 32) == 0) {
		CVI_INFO_CIPHER("Multi-package test PASSED\n");
	} else {
		CVI_ERR_CIPHER("Multi-package test FAILED - data mismatch\n");
		ret = CVI_FAILURE;
	}
	
	// Destroy handle
	CVI_UNF_CIPHER_DestroyHandle(hCipher);
	
	return ret;
}

// Main test function
int sample_cipher(void)
{
	CVI_S32 ret = CVI_SUCCESS;
	
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	
	CVI_INFO_CIPHER("=== CVI Cipher Test Started ===\n");
	
	// Initialize Cipher
	ret = CVI_UNF_CIPHER_Init();
	CHECK_RESULT(ret, "CVI_UNF_CIPHER_Init failed, ret=0x%x\n", ret);
	CVI_INFO_CIPHER("CVI_UNF_CIPHER_Init() success\n");
	
	// Test AES-128 CBC
	ret = test_aes_128_cbc();
	if (ret != CVI_SUCCESS) {
		CVI_ERR_CIPHER("AES-128 CBC test failed\n");
		goto cleanup;
	}
	
	// Test AES-256 ECB
	ret = test_aes_256_ecb();
	if (ret != CVI_SUCCESS) {
		CVI_ERR_CIPHER("AES-256 ECB test failed\n");
		goto cleanup;
	}
	
	// Test SHA256 hash
	ret = test_hash_sha256();
	if (ret != CVI_SUCCESS) {
		CVI_ERR_CIPHER("SHA256 hash test failed\n");
		goto cleanup;
	}
	
	// Test random number generation
	ret = test_random_number();
	if (ret != CVI_SUCCESS) {
		CVI_ERR_CIPHER("Random number test failed\n");
		goto cleanup;
	}
	
	// Test multi-package encryption and decryption
	ret = test_multi_package_encrypt();
	if (ret != CVI_SUCCESS) {
		CVI_ERR_CIPHER("Multi-package test failed\n");
		goto cleanup;
	}
	
	CVI_INFO_CIPHER("=== All Cipher Tests PASSED ===\n");
	
cleanup:
	// Deinitialize Cipher
	CVI_UNF_CIPHER_DeInit();
	CVI_INFO_CIPHER("CVI_UNF_CIPHER_DeInit() completed\n");
	
	return ret;
} 