#ifndef WORDSFIXTURE_H
#define WORDSFIXTURE_H

// This file is part of CDManager
//
// CDManager is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CDManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CDManager.  If not, see <http://www.gnu.org/licenses/>.

#include <sys/shm.h>

#include <initializer_list>
#include <string>
#include <vector>

#include "Words.h"

/**Fixture creating the (shared memory holding the) words for a test and
 * removing it afterwards
 */
struct WordsFixture {
    explicit WordsFixture(std::initializer_list<const char*> articles = {}, std::initializer_list<const char*> names = {}) {
        Words::create();
        for (const char* article : articles)
            Words::addArticle(article);
        for (const char* name : names)
            Words::addName2Ignore(name);
    }

    ~WordsFixture() {
        // Words::destroy only detaches the segments; remove them, so the tests don't leak them
        const int key(Words::getMemoryKey());
        const int keyValues(valuesKey());
        Words::destroy();
        shmctl(keyValues, IPC_RMID, nullptr);
        shmctl(key, IPC_RMID, nullptr);
    }

    WordsFixture(const WordsFixture&) = delete;
    WordsFixture& operator=(const WordsFixture&) = delete;

    /// Returns the key of the shared memory segment holding the words themselves
    static int valuesKey() {
        const void* mem(shmat(Words::getMemoryKey(), nullptr, SHM_RDONLY));
        if (mem == reinterpret_cast<void*>(-1))
            return -1;
        const int key(static_cast<const Words::values*>(mem)->valuesKey);
        shmdt(mem);
        return key;
    }

    /// Returns all stored names
    static std::vector<std::string> names() {
        Collector collector;
        Words::forEachName(0, Words::cNames(), collector, &Collector::add);
        return collector.words;
    }

    /// Returns all stored articles
    static std::vector<std::string> articles() {
        Collector collector;
        Words::forEachArticle(0, Words::cArticles(), collector, &Collector::add);
        return collector.words;
    }

  private:
    struct Collector {
        std::vector<std::string> words;
        void add(const char* word) { words.emplace_back(word); }
    };
};

#endif
