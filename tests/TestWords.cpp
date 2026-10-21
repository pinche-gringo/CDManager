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
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Words.h"
#include "WordsFixture.h"

#define BOOST_TEST_MODULE Words
#include "UnitTest.h"

namespace {

/// Returns the passed number as (at least 3 characters long) word, like "a07"
std::string makeWord(unsigned int nr) { return std::format("a{:02}", nr); }

/// Returns the passed number (< 3844) as word of two characters (sorted like the numbers)
std::string makeShortWord(unsigned int nr) {
    static constexpr std::string_view DIGITS("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
    return {DIGITS[nr / DIGITS.size()], DIGITS[nr % DIGITS.size()]};
}

} // namespace

BOOST_AUTO_TEST_SUITE(Management)

BOOST_AUTO_TEST_CASE(CreateAndDestroy) {
    BOOST_TEST(!Words::areAvailable());
    {
        WordsFixture words;
        BOOST_TEST(Words::areAvailable());
        BOOST_TEST(Words::getMemoryKey() != -1);
        BOOST_TEST(Words::cArticles() == 0U);
        BOOST_TEST(Words::cNames() == 0U);
    }
    BOOST_TEST(!Words::areAvailable());
    BOOST_TEST(Words::getMemoryKey() == -1);
}

BOOST_AUTO_TEST_CASE(CreateTwiceKeepsTheWords) {
    WordsFixture words({"The"});
    const int key(Words::getMemoryKey());
    Words::create();
    BOOST_TEST(Words::getMemoryKey() == key);
    BOOST_TEST(Words::cArticles() == 1U);
}

// CDManager creates the words; CDWriter (a child process) accesses them via the key
BOOST_AUTO_TEST_CASE(AccessFromOtherProcess) {
    WordsFixture words({"The", "Der"}, {"Mr", "Dr"});
    const int key(Words::getMemoryKey());

    const pid_t child(fork());
    BOOST_REQUIRE(child != -1);
    if (!child) {
        int rc(1);
        try {
            Words::access(key);
            if (Words::areAvailable() && (Words::cArticles() == 2) && (Words::cNames() == 2)
                && (Words::removeArticle("The Matrix") == "Matrix") && (Words::removeNames("Dr No") == "No"))
                rc = 0;
            Words::destroy();
        }
        catch (...) {
        }
        _exit(rc);
    }

    int status(0);
    BOOST_REQUIRE(waitpid(child, &status, 0) == child);
    BOOST_TEST(WIFEXITED(status));
    BOOST_TEST(WEXITSTATUS(status) == 0);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Storing)

BOOST_AUTO_TEST_CASE(NamesAreSorted) {
    WordsFixture words({}, {"Mrs", "Dr", "Prof", "Mr", "Sir"});
    BOOST_TEST(Words::cNames() == 5U);
    BOOST_TEST(Words::cArticles() == 0U);
    BOOST_TEST(WordsFixture::names() == (std::vector<std::string> {"Dr", "Mr", "Mrs", "Prof", "Sir"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(ArticlesAreSorted) {
    WordsFixture words({"The", "Die", "Der", "Las", "El", "Das", "A"});
    BOOST_TEST(Words::cArticles() == 7U);
    BOOST_TEST(Words::cNames() == 0U);
    BOOST_TEST(WordsFixture::articles() == (std::vector<std::string> {"A", "Das", "Der", "Die", "El", "Las", "The"}),
               boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(NamesAndArticlesAreSeparated) {
    WordsFixture words({"The", "A"}, {"Mr", "Dr"});
    BOOST_TEST(WordsFixture::articles() == (std::vector<std::string> {"A", "The"}), boost::test_tools::per_element());
    BOOST_TEST(WordsFixture::names() == (std::vector<std::string> {"Dr", "Mr"}), boost::test_tools::per_element());
}

BOOST_AUTO_TEST_CASE(Utf8Words) {
    WordsFixture words({"Él", "Los"}, {"Señor", "Herr"});
    BOOST_TEST(WordsFixture::articles() == (std::vector<std::string> {"Los", "Él"}), boost::test_tools::per_element());
    BOOST_TEST(WordsFixture::names() == (std::vector<std::string> {"Herr", "Señor"}), boost::test_tools::per_element());
    BOOST_TEST(Words::removeArticle("Él Mariachi") == "Mariachi");
    BOOST_TEST(Words::removeNames("Señor Smith") == "Smith");
}

// Every position hint (right or wrong) must keep the words sorted
BOOST_AUTO_TEST_CASE(PositionHintsKeepTheOrder) {
    const std::vector<std::string> input {"m", "c", "x", "a", "q", "e", "z", "b", "n", "d"};
    std::vector<std::string> expected(input);
    std::ranges::sort(expected);

    for (unsigned int hint : {0U, 1U, 2U, 3U, 5U, 9U, 100U, static_cast<unsigned int>(Words::POS_END)}) {
        BOOST_TEST_CONTEXT("Hint " << hint) {
            WordsFixture words;
            for (const auto& word : input) {
                Words::addName2Ignore(word, hint);
                Words::addArticle(word, hint);
            }
            BOOST_TEST(WordsFixture::names() == expected, boost::test_tools::per_element());
            BOOST_TEST(WordsFixture::articles() == expected, boost::test_tools::per_element());
        }
    }
}

// Appending sorted words with the right hint (as the word dialog does when saving)
BOOST_AUTO_TEST_CASE(AppendWithHint) {
    WordsFixture words;
    const std::vector<std::string> sorted {"a", "b", "c", "d"};
    for (unsigned int i(0); i < sorted.size(); ++i) {
        Words::addName2Ignore(sorted[i], i);
        Words::addArticle(sorted[i], i);
    }
    BOOST_TEST(WordsFixture::names() == sorted, boost::test_tools::per_element());
    BOOST_TEST(WordsFixture::articles() == sorted, boost::test_tools::per_element());
}

// Regression: The terminating \0 of the words wasn't stored (so they ran into the following memory)
BOOST_AUTO_TEST_CASE(WordsAreTerminated) {
    WordsFixture words;

    // Fill the memory for the words with garbage
    const int key(WordsFixture::valuesKey());
    shmid_ds stat {};
    BOOST_REQUIRE(shmctl(key, IPC_STAT, &stat) == 0);
    void* values(shmat(key, nullptr, 0));
    BOOST_REQUIRE(values != reinterpret_cast<void*>(-1));
    std::memset(values, 'X', stat.shm_segsz);
    shmdt(values);

    Words::addName2Ignore("Mr");
    Words::addName2Ignore("Dr");
    Words::addArticle("The");
    BOOST_TEST(WordsFixture::names() == (std::vector<std::string> {"Dr", "Mr"}), boost::test_tools::per_element());
    BOOST_TEST(WordsFixture::articles() == std::vector<std::string> {"The"}, boost::test_tools::per_element());
    BOOST_TEST(Words::removeArticle("The Matrix") == "Matrix");
    BOOST_TEST(Words::removeNames("Mr Bean") == "Bean");
}

// If all entries are used (before the memory for the words), adding throws and doesn't change the stored words
BOOST_AUTO_TEST_CASE(TooManyWords) {
    WordsFixture words;
    unsigned int count(0);
    BOOST_CHECK_THROW(
        while (count < 3844) {
            Words::addName2Ignore(makeShortWord(count));
            ++count;
        },
        std::length_error);
    BOOST_TEST(count > 1000U);
    BOOST_TEST(Words::cNames() == count);
    BOOST_CHECK_THROW(Words::addArticle("The"), std::length_error);
    BOOST_TEST(Words::cArticles() == 0U);

    const auto names(WordsFixture::names());
    BOOST_TEST(std::ranges::is_sorted(names));
    BOOST_TEST((std::ranges::adjacent_find(names) == names.end()));
}

// If the memory for the words is used up, adding throws and doesn't change the stored words
BOOST_AUTO_TEST_CASE(TooLongWords) {
    WordsFixture words;
    unsigned int count(0);
    BOOST_CHECK_THROW(
        while (count < 100000) {
            Words::addArticle(makeWord(count) + std::string(200, 'x'));
            ++count;
        },
        std::length_error);
    BOOST_TEST(count > 0U);
    BOOST_TEST(Words::cArticles() == count);
    BOOST_TEST(WordsFixture::articles().back() == makeWord(count - 1) + std::string(200, 'x'));

    // Short words might still fit
    BOOST_CHECK_THROW(Words::addName2Ignore(std::string(1000, 'y')), std::length_error);
    BOOST_TEST(Words::cNames() == 0U);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE(Removing)

BOOST_AUTO_TEST_CASE(RemoveArticle) {
    WordsFixture words({"The", "Der", "Die", "El", "A"});
    BOOST_TEST(Words::removeArticle("The Matrix") == "Matrix");
    BOOST_TEST(Words::removeArticle("Der Untergang") == "Untergang");
    BOOST_TEST(Words::removeArticle("A Beautiful Mind") == "Beautiful Mind");
    BOOST_TEST(Words::removeArticle("El Mariachi") == "Mariachi");
}

BOOST_AUTO_TEST_CASE(RemoveArticleOnlyAsWholeWord) {
    WordsFixture words({"The", "A"});
    BOOST_TEST(Words::removeArticle("Theater") == "Theater");
    BOOST_TEST(Words::removeArticle("Alien") == "Alien");
    BOOST_TEST(Words::removeArticle("Matrix") == "Matrix");
    BOOST_TEST(Words::removeArticle("Into the Wild") == "Into the Wild");
}

BOOST_AUTO_TEST_CASE(RemoveArticleOnlyOnce) {
    WordsFixture words({"The"});
    BOOST_TEST(Words::removeArticle("The The") == "The");
}

BOOST_AUTO_TEST_CASE(NameConsistingOfArticle) {
    WordsFixture words({"The", "A"});
    BOOST_TEST(Words::removeArticle("The") == "The");
    BOOST_TEST(Words::removeArticle("A") == "A");
    BOOST_TEST(Words::removeArticle("") == "");
}

BOOST_AUTO_TEST_CASE(RemoveArticleSkipsSeparators) {
    WordsFixture words({"The"});
    BOOST_TEST(Words::removeArticle("The-Matrix") == "Matrix");
    BOOST_TEST(Words::removeArticle("The  Matrix") == "Matrix");
    BOOST_TEST(Words::removeArticle("The - 'Matrix'") == "Matrix'");
}

BOOST_AUTO_TEST_CASE(RemoveNames) {
    WordsFixture words({}, {"Mr", "Dr", "Sir"});
    BOOST_TEST(Words::removeNames("Mr Bean") == "Bean");
    BOOST_TEST(Words::removeNames("Sir Dr Who") == "Who");
    BOOST_TEST(Words::removeNames("Mrs Doubtfire") == "Mrs Doubtfire");
    BOOST_TEST(Words::removeNames("Mr") == "Mr");
}

BOOST_AUTO_TEST_CASE(RemoveInitials) {
    WordsFixture words({}, {"Mr"});
    BOOST_TEST(Words::removeNames("J. R. R. Tolkien") == "Tolkien");
    BOOST_TEST(Words::removeNames("Mr T. Smith") == "Smith");
    BOOST_TEST(Words::removeNames("Jo. Smith") == "Jo. Smith");
}

// Without any stored words nothing is removed
BOOST_AUTO_TEST_CASE(NoWords) {
    WordsFixture words;
    BOOST_TEST(Words::removeArticle("The Matrix") == "The Matrix");
    BOOST_TEST(Words::removeNames("Mr Bean") == "Mr Bean");
    BOOST_TEST(Words::removeNames("J. Smith") == "Smith");
}

BOOST_AUTO_TEST_SUITE_END()
