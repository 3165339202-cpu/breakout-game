#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <ctime>
#include <cstdio>
#include <cstring>

struct ScoreEntry {
    char name[32];
    int score;
    time_t timestamp;
};

class Leaderboard {
private:
    static const int MAX_ENTRIES = 10;
    ScoreEntry entries[MAX_ENTRIES];
    int count;
    const char* filename;

public:
    Leaderboard(const char* file) : count(0), filename(file) {
        Load();
    }

    void Load() {
        FILE* f = fopen(filename, "r");
        if (f) {
            count = 0;
            while (count < MAX_ENTRIES &&
                   fscanf(f, "%31s %d %ld",
                          entries[count].name,
                          &entries[count].score,
                          &entries[count].timestamp) == 3) {
                count++;
            }
            fclose(f);
        }
    }

    void Save() {
        FILE* f = fopen(filename, "w");
        if (f) {
            for (int i = 0; i < count; i++) {
                fprintf(f, "%s %d %ld\n",
                        entries[i].name,
                        entries[i].score,
                        entries[i].timestamp);
            }
            fclose(f);
        }
    }

    int AddScore(const char* name, int score) {
        if (count >= MAX_ENTRIES &&
            score <= entries[count - 1].score)
            return 0;

        ScoreEntry newEntry;
        strncpy(newEntry.name, name, 31);
        newEntry.name[31] = '\0';
        newEntry.score = score;
        newEntry.timestamp = time(nullptr);

        int pos = 0;
        while (pos < count &&
               entries[pos].score >= score)
            pos++;

        if (count < MAX_ENTRIES) count++;

        for (int i = count - 1; i > pos; i--)
            entries[i] = entries[i - 1];

        entries[pos] = newEntry;

        Save();
        return pos + 1;
    }

    bool GetEntry(int rank, ScoreEntry& entry) {
        if (rank > 0 && rank <= count) {
            entry = entries[rank - 1];
            return true;
        }
        return false;
    }

    int GetCount() { return count; }
};

#endif