#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "../data.hpp"


namespace {

class WikiDataUnidirectional : public ::testing::Test {
protected:
  void SetUp() {
    data.links.resize(4);
    data.add_link_unsafe(0, 1, true);
    data.add_link_unsafe(0, 2, true);
    data.add_link_unsafe(0, 3, true);
    data.add_link_unsafe(3, 0, true);
  }

  WikiData data;
};


TEST_F(WikiDataUnidirectional, PageLinksExist) {
  // Test whether the created links exist
  EXPECT_TRUE(data.outlink_exists(0, 1));
  EXPECT_TRUE(data.outlink_exists(0, 2));
  EXPECT_TRUE(data.outlink_exists(0, 3));
  EXPECT_TRUE(data.outlink_exists(3, 0));

  // Test that other links don't exist
  EXPECT_FALSE(data.outlink_exists(0, 0));
  EXPECT_FALSE(data.outlink_exists(1, 0));
  EXPECT_FALSE(data.outlink_exists(1, 1));
  EXPECT_FALSE(data.outlink_exists(1, 2));
  EXPECT_FALSE(data.outlink_exists(1, 3));
  EXPECT_FALSE(data.outlink_exists(2, 0));
  EXPECT_FALSE(data.outlink_exists(2, 1));
  EXPECT_FALSE(data.outlink_exists(2, 2));
  EXPECT_FALSE(data.outlink_exists(2, 3));
  EXPECT_FALSE(data.outlink_exists(3, 1));
  EXPECT_FALSE(data.outlink_exists(3, 2));
  EXPECT_FALSE(data.outlink_exists(3, 3));

  // Missing target links return false
  EXPECT_FALSE(data.outlink_exists(0, 4));
  EXPECT_FALSE(data.outlink_exists(3, 4));
  EXPECT_FALSE(data.outlink_exists(0, -1));
  EXPECT_FALSE(data.outlink_exists(0, -1));

  // Missing source links throw std::runtime_error
  EXPECT_THROW(data.outlink_exists(4, 0), std::runtime_error);
  EXPECT_THROW(data.outlink_exists(-1, 0), std::runtime_error);

  EXPECT_NO_THROW(data.check_articleid_linkdb(0));
  EXPECT_NO_THROW(data.check_articleid_linkdb(3));
  EXPECT_THROW(data.check_articleid_linkdb(4), std::runtime_error);
};


TEST_F(WikiDataUnidirectional, PagelinkExportAndHelpers) {
  vector<WikiData::Pagelink> links = data.get_links(0);
  
  ASSERT_THAT(links, ::testing::ElementsAre(
        WikiData::to_pagelink(1, true, false),
        WikiData::to_pagelink(2, true, false),
        WikiData::to_pagelink(3, true, false)));

  EXPECT_EQ(1, WikiData::to_ArticleID(links[0]));
  EXPECT_EQ(2, WikiData::to_ArticleID(links[1]));
  EXPECT_EQ(3, WikiData::to_ArticleID(links[2]));

  EXPECT_TRUE(WikiData::is_outgoing(links[0]));
  EXPECT_TRUE(WikiData::is_outgoing(links[1]));
  EXPECT_TRUE(WikiData::is_outgoing(links[2]));
  
  EXPECT_FALSE(WikiData::is_incoming(links[0]));
  EXPECT_FALSE(WikiData::is_incoming(links[1]));
  EXPECT_FALSE(WikiData::is_incoming(links[2]));  

  EXPECT_TRUE(WikiData::is_link_to_article(links[0], 1));
  EXPECT_TRUE(WikiData::is_link_to_article(links[1], 2));
  EXPECT_TRUE(WikiData::is_link_to_article(links[2], 3));


  /// Same test with the third link
  links = data.get_links(3);

  ASSERT_THAT(links, ::testing::ElementsAre(
        WikiData::to_pagelink(0, true, false)));

  EXPECT_TRUE(WikiData::is_outgoing(links[0]));
  EXPECT_EQ(0, WikiData::to_ArticleID(links[0]));
  EXPECT_FALSE(WikiData::is_incoming(links[0]));
  EXPECT_TRUE(WikiData::is_link_to_article(links[0], 0));
};


class WikiDataBidirectional : public ::testing::Test {
protected:
  void SetUp() {
    data.links.resize(4);
    data.add_link_unsafe(0, 1, true);
    data.add_link_unsafe(1, 0, false);

    data.add_link_unsafe(0, 2, true);
    data.add_link_unsafe(2, 0, false);
  
    data.add_link_unsafe(0, 3, true);
    data.add_link_unsafe(3, 0, false);

    data.add_link_unsafe(3, 0, true);
    data.add_link_unsafe(0, 3, false);
  }

  WikiData data;
};


TEST_F(WikiDataBidirectional, PageLinksExist) {
  /** This part is identical to WikiDataUnidirectional */
  // Test whether the created links exist
  EXPECT_TRUE(data.outlink_exists(0, 1));
  EXPECT_TRUE(data.outlink_exists(0, 2));
  EXPECT_TRUE(data.outlink_exists(0, 3));
  EXPECT_TRUE(data.outlink_exists(3, 0));

  // Test that other links don't exist
  EXPECT_FALSE(data.outlink_exists(0, 0));
  EXPECT_FALSE(data.outlink_exists(1, 0));
  EXPECT_FALSE(data.outlink_exists(1, 1));
  EXPECT_FALSE(data.outlink_exists(1, 2));
  EXPECT_FALSE(data.outlink_exists(1, 3));
  EXPECT_FALSE(data.outlink_exists(2, 0));
  EXPECT_FALSE(data.outlink_exists(2, 1));
  EXPECT_FALSE(data.outlink_exists(2, 2));
  EXPECT_FALSE(data.outlink_exists(2, 3));
  EXPECT_FALSE(data.outlink_exists(3, 1));
  EXPECT_FALSE(data.outlink_exists(3, 2));
  EXPECT_FALSE(data.outlink_exists(3, 3));

  // Missing target links return false
  EXPECT_FALSE(data.outlink_exists(0, 4));
  EXPECT_FALSE(data.outlink_exists(3, 4));
  EXPECT_FALSE(data.outlink_exists(0, -1));
  EXPECT_FALSE(data.outlink_exists(0, -1));

  // Missing source links throw std::runtime_error
  EXPECT_THROW(data.outlink_exists(4, 0), std::runtime_error);
  EXPECT_THROW(data.outlink_exists(-1, 0), std::runtime_error);

  EXPECT_NO_THROW(data.check_articleid_linkdb(0));
  EXPECT_NO_THROW(data.check_articleid_linkdb(3));
  EXPECT_THROW(data.check_articleid_linkdb(4), std::runtime_error);
};


TEST_F(WikiDataBidirectional, PagelinkExportAndHelpers) {
  vector<WikiData::Pagelink> links = data.get_links(0, true, true);
  
  // only the link to number 3 "comes back":
  ASSERT_THAT(links, ::testing::ElementsAre(
        WikiData::to_pagelink(1, true, false),
        WikiData::to_pagelink(2, true, false),
        WikiData::to_pagelink(3, true, true)));

  EXPECT_EQ(1, WikiData::to_ArticleID(links[0]));
  EXPECT_EQ(2, WikiData::to_ArticleID(links[1]));
  EXPECT_EQ(3, WikiData::to_ArticleID(links[2]));

  EXPECT_TRUE(WikiData::is_outgoing(links[0]));
  EXPECT_TRUE(WikiData::is_outgoing(links[1]));
  EXPECT_TRUE(WikiData::is_outgoing(links[2]));
  
  EXPECT_FALSE(WikiData::is_incoming(links[0]));
  EXPECT_FALSE(WikiData::is_incoming(links[1]));
  EXPECT_TRUE(WikiData::is_incoming(links[2]));  

  EXPECT_TRUE(WikiData::is_link_to_article(links[0], 1));
  EXPECT_TRUE(WikiData::is_link_to_article(links[1], 2));
  EXPECT_TRUE(WikiData::is_link_to_article(links[2], 3));


  /// Same test with ArticleID = 3
  /// Expect a bi-directional link
  links = data.get_links(3, true, true);

  ASSERT_THAT(links, ::testing::ElementsAre(
        WikiData::to_pagelink(0, true, true)));

  EXPECT_TRUE(WikiData::is_outgoing(links[0]));
  EXPECT_EQ(0, WikiData::to_ArticleID(links[0]));
  EXPECT_TRUE(WikiData::is_incoming(links[0]));
  EXPECT_TRUE(WikiData::is_link_to_article(links[0], 0));

  /// With page 1, we expect only an incoming link
  links = data.get_links(1, true, true);

  ASSERT_THAT(links, ::testing::ElementsAre(
        WikiData::to_pagelink(0, false, true)));

  EXPECT_FALSE(WikiData::is_outgoing(links[0]));
  EXPECT_TRUE(WikiData::is_incoming(links[0]));
  EXPECT_EQ(0, WikiData::to_ArticleID(links[0]));
  EXPECT_TRUE(WikiData::is_link_to_article(links[0], 0));
};

}; // namespace

int main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
};
