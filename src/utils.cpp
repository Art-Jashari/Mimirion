#include "../include/utils.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <openssl/sha.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <zlib.h>
#include <cstring>
#include <sys/types.h>
#include <pwd.h>
#include <unistd.h>

namespace mimirion {
namespace utils {

std::string sha256(const std::string& data) {
    unsigned int length = SHA256_DIGEST_LENGTH;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    
    // Use EVP interface instead of deprecated SHA256 direct calls
    const EVP_MD* md = EVP_sha256();
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    
    EVP_DigestInit_ex(ctx, md, nullptr);
    EVP_DigestUpdate(ctx, data.c_str(), data.size());
    EVP_DigestFinal_ex(ctx, hash, &length);
    EVP_MD_CTX_free(ctx);
    
    std::stringstream ss;
    for(unsigned int i = 0; i < length; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string sha256File(const fs::path& path) {
    if (!fs::exists(path) || !fs::is_regular_file(path)) {
        return "";
    }
    
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "";
    }
    
    // Use EVP interface instead of deprecated SHA256 direct calls
    unsigned int length = SHA256_DIGEST_LENGTH;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    const EVP_MD* md = EVP_sha256();
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    
    EVP_DigestInit_ex(ctx, md, nullptr);
    
    char buffer[4096];
    while (file.read(buffer, sizeof(buffer))) {
        EVP_DigestUpdate(ctx, buffer, static_cast<size_t>(file.gcount()));
    }
    
    // Don't forget the last chunk if file size is not a multiple of the buffer size
    if (file.gcount() > 0) {
        EVP_DigestUpdate(ctx, buffer, static_cast<size_t>(file.gcount()));
    }
    
    EVP_DigestFinal_ex(ctx, hash, &length);
    EVP_MD_CTX_free(ctx);
    
    std::stringstream ss;
    for(unsigned int i = 0; i < length; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}

std::string getUserName() {
    // Try to get from environment
    const char* name = getenv("GIT_AUTHOR_NAME");
    if (name && *name) {
        return name;
    }
    
    // Try to get from git config
    FILE* pipe = popen("git config --get user.name 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        std::string result = "";
        while (!feof(pipe)) {
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
        }
        pclose(pipe);
        
        // Remove trailing newline
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        
        if (!result.empty()) {
            return result;
        }
    }
    
    // Try to get from system
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        return pw->pw_gecos;
    }
    
    // Fallback
    return "Unknown User";
}

std::string getUserEmail() {
    // Try to get from environment
    const char* email = getenv("GIT_AUTHOR_EMAIL");
    if (email && *email) {
        return email;
    }
    
    // Try to get from git config
    FILE* pipe = popen("git config --get user.email 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        std::string result = "";
        while (!feof(pipe)) {
            if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
                result += buffer;
            }
        }
        pclose(pipe);
        
        // Remove trailing newline
        if (!result.empty() && result.back() == '\n') {
            result.pop_back();
        }
        
        if (!result.empty()) {
            return result;
        }
    }
    
    // Try to get from system
    struct passwd* pw = getpwuid(getuid());
    if (pw) {
        // Try to construct email from username and hostname
        char hostname[1024];
        if (gethostname(hostname, sizeof(hostname)) == 0) {
            return std::string(pw->pw_name) + "@" + hostname;
        }
        return pw->pw_name;
    }
    
    // Fallback
    return "user@localhost";
}

std::string formatTimestamp(const std::chrono::system_clock::time_point& timestamp) {
    std::time_t time = std::chrono::system_clock::to_time_t(timestamp);
    std::tm* tm = std::gmtime(&time);
    
    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", tm);
    
    return std::string(buffer);
}

std::chrono::system_clock::time_point parseTimestamp(const std::string& str) {
    std::tm tm = {};
    std::istringstream ss(str);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string compress(const std::string& data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (deflateInit(&zs, Z_BEST_COMPRESSION) != Z_OK) {
        return "";
    }
    
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
    zs.avail_in = data.size();
    
    int ret;
    char outbuffer[32768];
    std::string outstring;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        
        ret = deflate(&zs, Z_FINISH);
        
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);
    
    deflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {
        return "";
    }
    
    return outstring;
}

std::string decompress(const std::string& data) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    
    if (inflateInit(&zs) != Z_OK) {
        return "";
    }
    
    zs.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));
    zs.avail_in = data.size();
    
    int ret;
    char outbuffer[32768];
    std::string outstring;
    
    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);
        
        ret = inflate(&zs, 0);
        
        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);
    
    inflateEnd(&zs);
    
    if (ret != Z_STREAM_END) {
        return "";
    }
    
    return outstring;
}

std::string readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return "";
    }
    
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::string buffer(size, '\0');
    file.read(&buffer[0], size);
    
    return buffer;
}

bool writeFile(const fs::path& path, const std::string& contents) {
    // Create parent directory if it doesn't exist
    fs::create_directories(path.parent_path());
    
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    file.write(contents.data(), contents.size());
    return file.good();
}

bool createDirectory(const fs::path& path) {
    try {
        return fs::create_directories(path);
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Failed to create directory: " << e.what() << std::endl;
        return false;
    }
}

std::vector<std::string> split(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(s);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

std::string join(const std::vector<std::string>& strings, const std::string& delimiter) {
    std::ostringstream oss;
    
    for (size_t i = 0; i < strings.size(); ++i) {
        oss << strings[i];
        if (i < strings.size() - 1) {
            oss << delimiter;
        }
    }
    
    return oss.str();
}

bool isBinaryFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return false;
    }
    
    char buffer[4096];
    size_t bytesRead = file.read(buffer, sizeof(buffer)).gcount();
    
    // Check for null bytes, which are common in binary files
    for (size_t i = 0; i < bytesRead; ++i) {
        if (buffer[i] == '\0') {
            return true;
        }
    }
    
    // Check if the file contains non-printable characters
    for (size_t i = 0; i < bytesRead; ++i) {
        unsigned char c = static_cast<unsigned char>(buffer[i]);
        if (c < 32 && c != '\n' && c != '\r' && c != '\t') {
            return true;
        }
    }
    
    return false;
}

std::string base64Encode(const std::string& data) {
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;
    
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    
    // Do not use newlines
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    
    BIO_write(bio, data.c_str(), data.length());
    BIO_flush(bio);
    
    BIO_get_mem_ptr(bio, &bufferPtr);
    std::string result(bufferPtr->data, bufferPtr->length);
    
    BIO_free_all(bio);
    
    return result;
}

std::string base64Decode(const std::string& encoded) {
    BIO *bio, *b64;
    
    int decodedLength = encoded.length();
    char* buffer = new char[decodedLength + 1];
    buffer[decodedLength] = '\0';
    
    bio = BIO_new_mem_buf(encoded.c_str(), -1);
    b64 = BIO_new(BIO_f_base64());
    
    // Do not use newlines
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    
    bio = BIO_push(b64, bio);
    
    int length = BIO_read(bio, buffer, decodedLength);
    std::string result(buffer, length);
    
    delete[] buffer;
    BIO_free_all(bio);
    
    return result;
}

} // namespace utils
} // namespace mimirion
