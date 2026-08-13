#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>

struct HighScoreEntry
{
    std::string name;   // 3 characters
    int         score;
};

// Singleton owns the on-disk top-5 leaderboard.
// Call SetFilePath() + Load() once at startup, Save() after AddEntry().
class HighScoreManager
{
public:
    static HighScoreManager& GetInstance()
    {
        static HighScoreManager inst;
        return inst;
    }

    void SetFilePath(const std::string& path) { m_filePath = path; }

    void Load()
    {
        m_entries.clear();
        std::ifstream f(m_filePath);
        if (!f.is_open()) return;   // no file yet starts empty
        std::string line;
        while (std::getline(f, line))
        {
            if (line.empty()) continue;
            std::istringstream ss(line);
            HighScoreEntry e;
            ss >> e.name >> e.score;
            if (!e.name.empty() && e.score >= 0)
                m_entries.push_back(e);
        }
        Sort();
        if (m_entries.size() > MAX_ENTRIES)
            m_entries.resize(MAX_ENTRIES);
    }

    void Save() const
    {
        std::ofstream f(m_filePath);
        for (const auto& e : m_entries)
            f << e.name << " " << e.score << "\n";
    }

    // Insert a new entry, keep only the top MAX_ENTRIES scores, then sort.
    void AddEntry(const std::string& name, int score)
    {
        m_entries.push_back({ name, score });
        Sort();
        if (m_entries.size() > MAX_ENTRIES)
            m_entries.resize(MAX_ENTRIES);
    }

    const std::vector<HighScoreEntry>& GetEntries() const { return m_entries; }

    // Returns the best score on record, or 0 if the file is empty.
    int GetTopScore() const
    {
        return m_entries.empty() ? 0 : m_entries.front().score;
    }

private:
    HighScoreManager() = default;

    std::string                m_filePath{ "Data/highscores.txt" };
    std::vector<HighScoreEntry> m_entries;
    static constexpr size_t    MAX_ENTRIES = 5;

    void Sort()
    {
        std::sort(m_entries.begin(), m_entries.end(),
            [](const HighScoreEntry& a, const HighScoreEntry& b)
            { return a.score > b.score; });
    }
};
