#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace mimirion {

namespace fs = std::filesystem;

namespace utils {

/**
 * @brief Calculate SHA-256 hash of a string
 * @param data Input string
 * @return SHA-256 hash as hexadecimal string
 */
std::string sha256(const std::string& data);

/**
 * @brief Calculate SHA-256 hash of a file
 * @param path Path to file
 * @return SHA-256 hash as hexadecimal string
 */
std::string sha256File(const fs::path& path);

/**
 * @brief Get current user's name from system
 * @return User name
 */
std::string getUserName();

/**
 * @brief Get current user's email from system
 * @return User email
 */
std::string getUserEmail();

/**
 * @brief Format a timestamp as ISO 8601 string
 * @param timestamp Timestamp
 * @return ISO 8601 formatted string
 */
std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp);

/**
 * @brief Parse an ISO 8601 timestamp string
 * @param str ISO 8601 formatted string
 * @return Parsed timestamp
 */
std::chrono::system_clock::time_point parseTimestamp(const std::string& str);

/**
 * @brief Compress data using zlib
 * @param data Input data
 * @return Compressed data
 */
std::string compress(const std::string& data);

/**
 * @brief Decompress data using zlib
 * @param data Compressed data
 * @return Decompressed data
 */
std::string decompress(const std::string& data);

/**
 * @brief Read entire file into string
 * @param path Path to file
 * @return File contents as string
 */
std::string readFile(const fs::path& path);

/**
 * @brief Write string to file
 * @param path Path to file
 * @param contents File contents
 * @return true if successful, false otherwise
 */
bool writeFile(const fs::path& path, const std::string& contents);

/**
 * @brief Create directory recursively
 * @param path Directory path
 * @return true if successful, false otherwise
 */
bool createDirectory(const fs::path& path);

/**
 * @brief Split string by delimiter
 * @param s Input string
 * @param delimiter Delimiter
 * @return Vector of substrings
 */
std::vector<std::string> split(const std::string& s, char delimiter);

/**
 * @brief Join strings with delimiter
 * @param strings Vector of strings
 * @param delimiter Delimiter
 * @return Joined string
 */
std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

/**
 * @brief Check if file is binary
 * @param path Path to file
 * @return true if binary, false if text
 */
bool isBinaryFile(const fs::path& path);

/**
 * @brief Base64 encode data
 * @param data Input data
 * @return Base64 encoded string
 */
std::string base64Encode(const std::string& data);

/**
 * @brief Base64 decode data
 * @param encoded Base64 encoded string
 * @return Decoded data
 */
std::string base64Decode(const std::string& encoded);

} // namespace utils
} // namespace mimirion
