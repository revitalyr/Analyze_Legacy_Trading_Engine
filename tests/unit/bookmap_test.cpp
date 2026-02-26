#include <gtest/gtest.h>

#include "core/bookmap.h"

static OrderBookListener listener;

TEST(BookMapTest, BooksBasic) {
    BookMap books;

    auto book = books.get("dummy");
    EXPECT_EQ(book, nullptr);

    book = books.getOrCreate("dummy", listener);
    EXPECT_NE(book, nullptr);

    auto book2 = books.getOrCreate("dummy", listener);
    EXPECT_EQ(book, book2);

    auto book3 = books.get("dummy");
    EXPECT_EQ(book, book2);
    EXPECT_EQ(book2, book3);
}

TEST(BookMapTest, Instruments) {
    BookMap books;
    
    auto book = books.getOrCreate("dummy", listener);
    EXPECT_NE(book, nullptr);
    
    // Check if book has instruments
    if (book && book->instruments().size() > 0) {
        EXPECT_EQ(book->instruments()[0], "dummy");
        EXPECT_EQ(book->instruments().size(), 1u);
    } else {
        EXPECT_EQ(book->instruments().size(), 0u);
    }
    
    // Check if book has exactly one instrument
    if (book && book->instruments().size() == 1) {
        EXPECT_EQ(book->instruments()[0], "dummy");
    } else {
        EXPECT_EQ(book->instruments().size(), 1u);
    }
}
