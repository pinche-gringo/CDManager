// PROJECT     : CDManager
// SUBSYSTEM   : Words
// REFERENCES  :
// TODO        :
// BUGS        :
// AUTHOR      : Markus Schwab
// CREATED     : 30.10.2004
// COPYRIGHT   : Copyright (C) 2004 - 2006, 2009 - 2011, 2015, 2026

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

#include <cdmgr-cfg.h>

#include <sys/shm.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <limits>
#include <map>
#include <memory>
#include <type_traits>

#include <glibmm/unicode.h>

#include <YGP/Process.h>
#include <YGP/Trace.h>

#include "Words.h"

#undef PAGE_SIZE
constexpr unsigned int PAGE_SIZE(4096);

int Words::_key(-1);

namespace {

/// Pointers to the attached shared memory segments of one process
struct WordPtrs {
    Words::values* info{nullptr};
    char* values{nullptr};
    std::size_t cbValues{0}; ///< Size of the segment holding the values
};
std::map<pid_t, std::unique_ptr<WordPtrs>> ptrs;

/// Value returned by shmat in case of an error
void* const SHM_FAILED(reinterpret_cast<void*>(-1));

/// Returns the shared memory pointers of the actual process
WordPtrs* currentPtrs() { return ptrs[YGP::Process::getPID()].get(); }

/// Type of the offsets of the words within the values
using Offset = std::remove_extent_t<decltype(Words::values::aOffsets)>;

/// Throws if a word of the passed length can't be stored (no free entry, the
/// values are full or its offset doesn't fit into an Offset)
void checkSpace(const WordPtrs& shMem, std::size_t bytes) {
    const Words::values& info(*shMem.info);
    if (((info.cNames + info.cArticles) >= info.maxEntries) || ((info.used + bytes + 1) > shMem.cbValues) ||
        (info.used > std::numeric_limits<Offset>::max()))
        throw std::length_error(strerror(ENOSPC));
}

} // namespace

//-----------------------------------------------------------------------------
/// Creates the memory for the reserved words.
/// \param words: Minimal number of reserved words
/// \remarks The words are stored in shared memory (to be accessible by other
///    processes
//-----------------------------------------------------------------------------
void Words::create(unsigned int words) {
    TRACE1("Words::create (unsigned int) - " << words);
    if ((_key != -1) || areAvailable())
        return;

    WordPtrs* shMem((ptrs[YGP::Process::getPID()] = std::make_unique<WordPtrs>()).get());

    unsigned int size(PAGE_SIZE);
    if ((sizeof(values) + (sizeof(Offset) * words)) > size)
        size = ((sizeof(Offset) * words) + PAGE_SIZE + sizeof(values)) & ~(PAGE_SIZE - 1);

    if (((_key = shmget(IPC_PRIVATE, size, IPC_CREAT | IPC_EXCL | 0600)) == -1) ||
        ((shMem->info = static_cast<values*>(shmat(_key, nullptr, 0))) == SHM_FAILED) ||
        ((shMem->info->valuesKey = shmget(IPC_PRIVATE, size << 1, IPC_CREAT | IPC_EXCL | 0600)) == -1) ||
        ((shMem->values = static_cast<char*>(shmat(shMem->info->valuesKey, nullptr, 0))) == SHM_FAILED)) {
        destroy();
        throw(std::invalid_argument(strerror(errno)));
    }

    shMem->cbValues = size << 1;
    shMem->info->cNames = shMem->info->cArticles = 0;
    shMem->info->used = 0;
    shMem->info->maxEntries = (size - sizeof(values)) / sizeof(Offset);
    TRACE1("Words::create (unsigned int) - Key: " << _key << '/' << shMem->info->maxEntries);
}

//-----------------------------------------------------------------------------
/// Gets access to the shared memory with the passed id
/// \param key: ID of shared memory
/// \pre Requires the shared memory to be already created
//-----------------------------------------------------------------------------
void Words::access(unsigned int key) {
    TRACE8("Words::access (unsigned int) - " << key);
    if (!key || areAvailable())
        return;

    WordPtrs* shMem((ptrs[YGP::Process::getPID()] = std::make_unique<WordPtrs>()).get());

    if (((shMem->info = static_cast<values*>(shmat(key, nullptr, 0))) == SHM_FAILED) ||
        ((shMem->values = static_cast<char*>(shmat(shMem->info->valuesKey, nullptr, 0))) == SHM_FAILED)) {
        destroy();
        throw(std::invalid_argument(strerror(errno)));
    }

    shmid_ds stat;
    if (shmctl(shMem->info->valuesKey, IPC_STAT, &stat) == -1) {
        destroy();
        throw(std::invalid_argument(strerror(errno)));
    }
    shMem->cbValues = stat.shm_segsz;
    TRACE1("Words::access (unsigned int) - Articles: " << shMem->info->cArticles << "; Names: " << shMem->info->cNames << ": "
                                                       << *shMem->values);
}

//-----------------------------------------------------------------------------
/// Checks if the Words are already available for the actual process
/// \returns bool: True, if the Words are available
//-----------------------------------------------------------------------------
bool Words::areAvailable() { return ptrs.contains(YGP::Process::getPID()); }

//-----------------------------------------------------------------------------
/// Frees the used shared memory
//-----------------------------------------------------------------------------
void Words::destroy() {
    const WordPtrs* shMem(currentPtrs());

    if (shMem->info && shMem->info != SHM_FAILED) {
        if (shMem->values && shMem->values != SHM_FAILED)
            shmdt(shMem->values);
        shmdt(shMem->info);
    }

    ptrs.erase(YGP::Process::getPID());
    if (ptrs.empty())
        _key = -1;
}

//-----------------------------------------------------------------------------
/// Moves values from the values-array to another position
/// \param start: Start position
/// \param end: End position
/// \param target: Target position
/// \pre start <= end
//-----------------------------------------------------------------------------
void Words::moveValues(unsigned int start, unsigned int end, unsigned int target) {
    TRACE1("Words::moveValues (3x unsigned int start) - [" << start << '-' << end << "] -> " << target
                                                           << "; Bytes: " << (end - start + 1) * sizeof(Offset));
    Words::values* shMem(currentPtrs()->info);
    memmove(shMem->aOffsets + target, shMem->aOffsets + start, (end - start + 1) * sizeof(Offset));
}

//-----------------------------------------------------------------------------
/// Binary search for the passed words in the passed range
/// \param values: Values to search
/// \param data: Stored data
/// \param start: First value to search
/// \param end: Last value to search
/// \param word: Word to search
/// \returns unsigned int: Position where to insert
/// \pre start <= end
/// \requires There must be at least one element in the array
//-----------------------------------------------------------------------------
unsigned int Words::binarySearch(values* values, char* data, unsigned int start, unsigned int end, const char* word) {
    while ((end - start) > 0) {
        const unsigned int middle(start + ((end - start) >> 1));
        contract_assert(strcmp(data + values->aOffsets[start], data + values->aOffsets[middle]) <= 0);

        if (strcmp(word, data + values->aOffsets[middle]) < 0)
            end = middle;
        else
            start = middle + 1;
    }
    TRACE9("Words::binarySearch (values*, char*, 2x unsigned int, const char*) - " << start);
    return start;
}

//-----------------------------------------------------------------------------
/// Adds a name to ignore
/// \param word: Word to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addName2Ignore(const Glib::ustring& word, unsigned int pos) {
    WordPtrs* shMem(currentPtrs());
    TRACE2("Words::addName2Ignore (const Glib::ustring&, unsigned int) - " << word << " to " << shMem->info->cNames);
    checkSpace(*shMem, word.bytes());

    // Try to respect the hint
    if (pos != POS_UNKNOWN) {
        if (pos > shMem->info->cNames)
            pos = shMem->info->cNames;

        TRACE9("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Checking pos " << pos);
        if (shMem->info->cNames) {
            if (!pos || (strcmp(shMem->values + shMem->info->aOffsets[pos - 1], word.c_str()) < 0)) {
                if (pos < shMem->info->cNames) {
                    if (strcmp(shMem->values + shMem->info->aOffsets[pos], word.c_str()) <= 0)
                        pos = POS_UNKNOWN;
                }
            }
            else
                pos = POS_UNKNOWN;
        }
    }

    // Hint didn't work or wasn't passed: Search for position to insert
    if (pos == POS_UNKNOWN) {
        if (shMem->info->cNames) {
            TRACE9("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Search: " << word);
            pos = binarySearch(shMem->info, shMem->values, 0, shMem->info->cNames, word.c_str());
        }
        else
            pos = 0;
    }

    if (pos < shMem->info->cNames)
        moveValues(pos, shMem->info->cNames - 1, pos + 1);

    TRACE1("Words::addName2Ignore (const Glib::ustring&, unsigned int) - Insert into " << pos);
    shMem->info->aOffsets[pos] = shMem->info->used;
    memcpy(shMem->values + shMem->info->used, word.c_str(), word.bytes());
    shMem->info->used += word.bytes() + 1;
    shMem->info->cNames++;
}

//-----------------------------------------------------------------------------
/// Adds an article to ignore
/// \param word: Article to ignore
/// \param pos: Hint of position, where to insert
//-----------------------------------------------------------------------------
void Words::addArticle(const Glib::ustring& word, unsigned int pos) {
    WordPtrs* shMem(currentPtrs());
    TRACE1("Words::addArticle (const Glib::ustring&, unsigned int) - " << word << " to " << shMem->info->cArticles);
    checkSpace(*shMem, word.bytes());

    // Try to respect the hint
    if (pos != POS_UNKNOWN) {
        pos = ((pos >= shMem->info->cArticles) ? shMem->info->maxEntries - 1
                                              : shMem->info->maxEntries - shMem->info->cArticles + pos);

        TRACE1("Words::addArticle (const Glib::ustring&, unsigned int) - Checking pos " << pos);
        if (shMem->info->cArticles) {
            TRACE1("Words::addArticle (const Glib::ustring&, unsigned int) - Comp: "
                   << strcmp(shMem->values + shMem->info->aOffsets[pos], word.c_str()));
            if (strcmp(shMem->values + shMem->info->aOffsets[pos], word.c_str()) < 0) {
                if (pos < (shMem->info->maxEntries - 1)) {
                    if (strcmp(shMem->values + shMem->info->aOffsets[pos + 1], word.c_str()) <= 0)
                        pos = POS_UNKNOWN;
                }
            }
            else
                pos = POS_UNKNOWN;
        }
    }

    // Hint didn't work or wasn't passed: Search for position to insert
    if (pos == POS_UNKNOWN) {
        if (shMem->info->cArticles) {
            TRACE1("Words::addArticles (const Glib::ustring&, unsigned int) - Search: " << word);
            // The word is inserted after pos (see below), so use the position before the found one
            pos = binarySearch(shMem->info, shMem->values, shMem->info->maxEntries - shMem->info->cArticles,
                               shMem->info->maxEntries, word.c_str()) - 1;
        }
        else
            pos = shMem->info->maxEntries - 1;
    }

    if (pos >= (shMem->info->maxEntries - shMem->info->cArticles))
        moveValues(shMem->info->maxEntries - shMem->info->cArticles, pos, shMem->info->maxEntries - shMem->info->cArticles - 1);

    TRACE1("Words::addArticle (const Glib::ustring&, unsigned int) - Insert into " << pos);
    shMem->info->aOffsets[pos] = shMem->info->used;
    memcpy(shMem->values + shMem->info->used, word.c_str(), word.bytes());
    shMem->info->used += word.bytes() + 1;
    shMem->info->cArticles++;
    TRACE1("Words::addArticle (const Glib::ustring&, unsigned int) - Counts " << shMem->info->cArticles << '/'
                                                                              << shMem->info->cNames);
}

//-----------------------------------------------------------------------------
/// Removes a leading article from the passed name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Name without article
//-----------------------------------------------------------------------------
Glib::ustring Words::removeArticle(const Glib::ustring& name) {
    TRACE9("Words::removeArticles (const Glib::ustring&) - " << name);
    const WordPtrs* shMem(currentPtrs());

    Glib::ustring word(getWord(name));
    if (word.size() != name.size() &&
        containsWord(shMem->info->maxEntries - shMem->info->cArticles, shMem->info->maxEntries, word)) {
        unsigned int pos(word.size());
        while ((pos < name.size()) && !Glib::Unicode::isalnum(name[pos]))
            ++pos;

        TRACE3("Words::removeArticles (const Glib::ustring&) - " << name << "->" << name.substr(pos));
        return name.substr(pos);
    }
    return name;
}

//-----------------------------------------------------------------------------
/// Removes a leading name from the passed name.
/// \param name: Name to manipulate
/// \returns Glib::ustring: Name without name
//-----------------------------------------------------------------------------
Glib::ustring Words::removeNames(const Glib::ustring& name) {
    TRACE9("Words::removeNames (const Glib::ustring&) - " << name);
    const WordPtrs* shMem(currentPtrs());
    Glib::ustring work(name);
    Glib::ustring word(getWord(work));
    while ((word.size() != work.size()) &&
           (((word.size() == 2) && (word[1] == '.')) || containsWord(0, shMem->info->cNames, word))) {
        unsigned int pos(word.size());
        while ((pos < work.size()) && !Glib::Unicode::isalnum(work[pos]))
            ++pos;

        work = work.substr(pos);
        word = getWord(work);
    }
    TRACE3("Words::removeName (const Glib::ustring&) - " << name << "->" << work);
    return work;
}

//-----------------------------------------------------------------------------
/// Returns the first word of the passed string
/// \param text: Text to extract the first word from
/// \returns Glib::ustring: Changed name
//-----------------------------------------------------------------------------
Glib::ustring Words::getWord(const Glib::ustring& text) {
    unsigned int i(-1U);
    while (++i < text.size())
        if (Glib::Unicode::isspace(text[i]) || (text[i] == '-'))
            break;

    TRACE9("Words::getWord (const Glib::ustring&) - '" << text.substr(0, i) << '\'');
    return text.substr(0, i);
}

//-----------------------------------------------------------------------------
/// Checks if the passed list contains the passed word
/// \param start: Start value
/// \param end: End value
/// \param word: Word to search for
/// \returns bool: True, if the word exists
//-----------------------------------------------------------------------------
bool Words::containsWord(unsigned int start, unsigned int end, const Glib::ustring& word) {
    TRACE9("Words::containsWord (2x unsigned int, const Glib::ustring& word) - [" << start << '-' << end << ']');
    const WordPtrs* shMem(currentPtrs());
    contract_assert(end <= shMem->info->maxEntries);
    TRACE9("Words::containsWord (2x unsigned int, const Glib::ustring& word) - " << shMem->values + shMem->info->aOffsets[start]);
    TRACE9("Words::containsWord (2x unsigned int, const Glib::ustring& word) - " << shMem->values + shMem->info->aOffsets[end - 1]);
    if (start < end) {
        unsigned int pos(binarySearch(shMem->info, shMem->values, start, end, word.c_str()));
        return ((pos != start) && (word == (shMem->values + shMem->info->aOffsets[pos - 1])));
    }
    else
        return false;
}

//-----------------------------------------------------------------------------
/// Returns the number of articles stored
/// \returns unsigned int: Number of articles stored
//-----------------------------------------------------------------------------
unsigned int Words::cArticles() {
    return currentPtrs()->info->cArticles;
}

//-----------------------------------------------------------------------------
/// Returns the number of names stored
/// \returns unsigned int: Number of names stored
//-----------------------------------------------------------------------------
unsigned int Words::cNames() {
    return currentPtrs()->info->cNames;
}

//-----------------------------------------------------------------------------
/// Returns the stored words for the current process
/// \returns unsigned int: Stored words
//-----------------------------------------------------------------------------
const char* Words::getValues() {
    return currentPtrs()->values;
}

//-----------------------------------------------------------------------------
/// Returns the stored values for the current process
/// \returns unsigned int: Stored values
//-----------------------------------------------------------------------------
Words::values* Words::getInfo() {
    return currentPtrs()->info;
}
