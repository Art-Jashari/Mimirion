#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace mimirion {

namespace fs = std::filesystem;

struct DiffHunk {
    int oldStart;     // Start line in old file
    int oldCount;     // Number of lines in old file
    int newStart;     // Start line in new file
    int newCount;     // Number of lines in new file
    std::vector<std::string> lines; // Diff lines with context
};

struct FileDiff {
    std::string oldFile;      // Old file path
    std::string newFile;      // New file path
    std::vector<DiffHunk> hunks; // Diff hunks
};

/**
 * @brief Class for creating and applying diffs between files
 */
class DiffEngine {
public:
    DiffEngine();
    
    /**
     * @brief Generate a diff between two files
     * @param oldFile Path to old file
     * @param newFile Path to new file
     * @param contextLines Number of context lines to include (default: 3)
     * @return FileDiff object representing the differences
     */
    FileDiff generateDiff(const fs::path& oldFile, const fs::path& newFile, int contextLines = 3) const;
    
    /**
     * @brief Generate a diff between two strings
     * @param oldContent Old content as string
     * @param newContent New content as string
     * @param contextLines Number of context lines to include (default: 3)
     * @return FileDiff object representing the differences
     */
    FileDiff generateDiffFromStrings(const std::string& oldContent, const std::string& newContent, 
                                 int contextLines = 3) const;
    
    /**
     * @brief Apply a diff to a file
     * @param diff FileDiff object to apply
     * @param target Path to target file
     * @return true if successful, false otherwise
     */
    bool applyDiff(const FileDiff& diff, const fs::path& target) const;
    
    /**
     * @brief Convert a diff to string representation
     * @param diff FileDiff object
     * @return String representation of the diff
     */
    std::string diffToString(const FileDiff& diff) const;
    
    /**
     * @brief Parse a diff from string representation
     * @param diffStr String representation of the diff
     * @return FileDiff object
     */
    FileDiff parseDiff(const std::string& diffStr) const;

private:
    std::vector<std::string> splitLines(const std::string& content) const;
    std::vector<DiffHunk> computeHunks(const std::vector<std::string>& oldLines, 
                                      const std::vector<std::string>& newLines, 
                                      int contextLines) const;
};

} // namespace mimirion
