#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

#include "url.h"
#include "bool.h"
#include "urlconnection.h"
#include "streamtokenizer.h"
#include "html-utils.h"
#include "hashset.h"

static void Welcome(const char *welcomeTextFileName);
static void FillStopWords(const char* stopWordsFileName, hashset *hashset);
static void BuildIndices(const char *feedsFileName, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static void ProcessFeed(const char *remoteDocumentName, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static void PullAllNewsItems(urlconnection *urlconn, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static bool GetNextItemTag(streamtokenizer *st);
static void ProcessSingleNewsItem(streamtokenizer *st, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static void ExtractElement(streamtokenizer *st, const char *htmlTag, char dataBuffer[], int bufferLength);
static void ParseArticle(const char *articleTitle, const char *articleDescription, const char *articleURL, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static void ScanArticle(streamtokenizer *st, const char *articleTitle, const char *unused, const char *articleURL, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static void QueryIndices(hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static void ProcessResponse(const char *word, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles);
static bool WordIsWellFormed(const char *word);

/**
 * Helpers
 */

static const signed long kHashMultiplier = -1664117991L;
static int StringHash(const void *elem, int numBuckets)  
{            
  char *s = *(char **)elem;
  int i;
  unsigned long hashcode = 0;
  
  for (i = 0; i < strlen(s); i++)  
    hashcode = hashcode * kHashMultiplier + tolower(s[i]);  
  
  return hashcode % numBuckets;                                
}

static int StringCompareFunction(const void *elemAddr1, const void *elemAddr2) {
  return strcmp(*(char **)elemAddr1, *(char **)elemAddr2);
}

static void StringFree(void *elem) {
  free(*(char **)elem);
}

/**
 * Data structures
 */

struct index {
  char *word;
  vector *articles;
};

struct articleInfo {
  char *articleTitle;
  char *articleURL;
};

struct articleCount {
  int count;
  struct articleInfo *info;
};

static int IndexHash(const void *elem, int numBuckets)  
{
  struct index *idx = (struct index *)elem;
  return StringHash(&idx->word, numBuckets);
}

static int IndexCompareFunction(const void *elemAddr1, const void *elemAddr2) {
  char *first = ((struct index *)elemAddr1)->word;
  char *second = ((struct index *)elemAddr2)->word;
  return StringCompareFunction(&first, &second);
}

static void IndexFree(void *elem) {
  struct index *idx = (struct index *)elem;
  printf("Freeing index \"%s\" of count %d\n", idx->word, VectorLength(idx->articles));
  VectorDispose(idx->articles);
  free(idx->articles);
  StringFree(&idx->word);
}

static int ArticleInfoHash(const void *elem, int numBuckets)  
{
  struct articleInfo *artI = *(struct articleInfo **)elem;
  return StringHash(&artI->articleTitle, numBuckets);
}

static int ArticleInfoCompareFunction(const void *elemAddr1, const void *elemAddr2) {
  // TODO: compare urls too
  char *first = (*(struct articleInfo **)elemAddr1)->articleTitle;
  char *second = (*(struct articleInfo **)elemAddr2)->articleTitle;
  return StringCompareFunction(&first, &second);
}

static void ArticleInfoFree(void *elem) {
  struct articleInfo *artI = *(struct articleInfo **)elem;
  printf("\tFreeing article info 0x%08lx \"%s\" \"%s\"\n", (long unsigned int)artI, artI->articleTitle, artI->articleURL);
  StringFree(&artI->articleTitle);
  StringFree(&artI->articleURL);
  free(artI);
  //free(artC);
}

static void ArticleCountFree(void *elem) {
  struct articleCount *artC = *(struct articleCount **)elem;
  /*
  struct articleInfo *artI = artC->info;
  printf("\tFreeing article 0x%08lx \"%s\" \"%s\"\n", (long unsigned int)artC, artI->articleTitle, artI->articleURL);
  StringFree(&artI->articleTitle);
  StringFree(&artI->articleURL);
  free(artI);
  */
  free(artC);
}

/**
 * Function: main
 * --------------
 * Serves as the entry point of the full application.
 * You'll want to update main to declare several hashsets--
 * one for stop words, another for previously seen urls, etc--
 * and pass them (by address) to BuildIndices and QueryIndices.
 * In fact, you'll need to extend many of the prototypes of the
 * supplied helpers functions to take one or more hashset *s.
 *
 * Think very carefully about how you're going to keep track of
 * all of the stop words, how you're going to keep track of
 * all the previously seen articles, and how you're going to 
 * map words to the collection of news articles where that
 * word appears.
 */

static const char *const kWelcomeTextFile = "/usr/class/cs107/assignments/assn-4-rss-news-search-data/welcome.txt";
static const char *const kStopWordsFile = "/usr/class/cs107/assignments/assn-4-rss-news-search-data/stop-words.txt";
static const char *const kDefaultFeedsFile = "/usr/class/cs107/assignments/assn-4-rss-news-search-data/rss-feeds.txt";
int main(int argc, char **argv)
{
  Welcome(kWelcomeTextFile);
  
  hashset stopWords;
  HashSetNew(&stopWords, sizeof(char *), 1009, StringHash, StringCompareFunction, StringFree);
  FillStopWords(kStopWordsFile, &stopWords);
  
  hashset indexedWords;
  HashSetNew(&indexedWords, sizeof(struct index), 10007, IndexHash, IndexCompareFunction, IndexFree);

  hashset indexedArticles;
  HashSetNew(&indexedArticles, sizeof(struct articleInfo *), 10007, ArticleInfoHash, ArticleInfoCompareFunction, ArticleInfoFree);

  BuildIndices((argc == 1) ? kDefaultFeedsFile : argv[1], &stopWords, &indexedWords, &indexedArticles);
  QueryIndices(&stopWords, &indexedWords, &indexedArticles);

  HashSetDispose(&indexedArticles);
  HashSetDispose(&indexedWords);
  HashSetDispose(&stopWords);
  return 0;
}

/** 
 * Function: Welcome
 * -----------------
 * Displays the contents of the specified file, which
 * holds the introductory remarks to be printed every time
 * the application launches.  This type of overhead may
 * seem silly, but by placing the text in an external file,
 * we can change the welcome text without forcing a recompilation and
 * build of the application.  It's as if welcomeTextFileName
 * is a configuration file that travels with the application.
 */
 
static const char *const kNewLineDelimiters = "\r\n";
static void Welcome(const char *welcomeTextFileName)
{
  FILE *infile;
  streamtokenizer st;
  char buffer[1024];
  
  infile = fopen(welcomeTextFileName, "r");
  assert(infile != NULL);    
  
  STNew(&st, infile, kNewLineDelimiters, true);
  while (STNextToken(&st, buffer, sizeof(buffer))) {
    printf("%s\n", buffer);
  }
  
  printf("\n");
  STDispose(&st); // remember that STDispose doesn't close the file, since STNew doesn't open one.. 
  fclose(infile);
}

/** 
 * Function: FillStopWords
 * -----------------
 */

static const char *const kWhiteSpaceCharacters = " \t\n\r";
static void FillStopWords(const char* stopWordsFileName, hashset *hashset)
{
  FILE *file;
  streamtokenizer st;
  char word[128];

  file = fopen(stopWordsFileName, "r");
  STNew(&st, file, kWhiteSpaceCharacters, true);
  while (STNextToken(&st, word, sizeof(word))) {
    char *s = strdup(word);
    HashSetEnter(hashset, &s);
  }
  STDispose(&st);
  fclose(file);
}

/**
 * Function: BuildIndices
 * ----------------------
 * As far as the user is concerned, BuildIndices needs to read each and every
 * one of the feeds listed in the specied feedsFileName, and for each feed parse
 * content of all referenced articles and store the content in the hashset of indices.
 * Each line of the specified feeds file looks like this:
 *
 *   <feed name>: <URL of remore xml document>
 *
 * Each iteration of the supplied while loop parses and discards the feed name (it's
 * in the file for humans to read, but our aggregator doesn't care what the name is)
 * and then extracts the URL.  It then relies on ProcessFeed to pull the remote
 * document and index its content.
 */

static void BuildIndices(const char *feedsFileName, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  FILE *infile;
  streamtokenizer st;
  char remoteFileName[1024];
  
  infile = fopen(feedsFileName, "r");
  assert(infile != NULL);
  STNew(&st, infile, kNewLineDelimiters, true);
  while (STSkipUntil(&st, ":") != EOF) { // ignore everything up to the first selicolon of the line
    STSkipOver(&st, ": ");		 // now ignore the semicolon and any whitespace directly after it
    STNextToken(&st, remoteFileName, sizeof(remoteFileName));   
    ProcessFeed(remoteFileName, stopWords, indexedWords, indexedArticles);
  }
  
  STDispose(&st);
  fclose(infile);
  printf("\n");
}


/**
 * Function: ProcessFeed
 * ---------------------
 * ProcessFeed locates the specified RSS document, and if a (possibly redirected) connection to that remote
 * document can be established, then PullAllNewsItems is tapped to actually read the feed.  Check out the
 * documentation of the PullAllNewsItems function for more information, and inspect the documentation
 * for ParseArticle for information about what the different response codes mean.
 */

static void ProcessFeed(const char *remoteDocumentName, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  url u;
  urlconnection urlconn;
  
  URLNewAbsolute(&u, remoteDocumentName);
  URLConnectionNew(&urlconn, &u);
  
  switch (urlconn.responseCode) {
      case 0: printf("Unable to connect to \"%s\".  Ignoring...", u.serverName);
              break;
  case 200: PullAllNewsItems(&urlconn, stopWords, indexedWords, indexedArticles);
                break;
      case 301: 
  case 302: ProcessFeed(urlconn.newUrl, stopWords, indexedWords, indexedArticles);
                break;
      default: printf("Connection to \"%s\" was established, but unable to retrieve \"%s\". [response code: %d, response message:\"%s\"]\n",
		      u.serverName, u.fileName, urlconn.responseCode, urlconn.responseMessage);
	       break;
  };
  
  URLConnectionDispose(&urlconn);
  URLDispose(&u);
}

/**
 * Function: PullAllNewsItems
 * --------------------------
 * Steps though the data of what is assumed to be an RSS feed identifying the names and
 * URLs of online news articles.  Check out "datafiles/sample-rss-feed.txt" for an idea of what an
 * RSS feed from the www.nytimes.com (or anything other server that syndicates is stories).
 *
 * PullAllNewsItems views a typical RSS feed as a sequence of "items", where each item is detailed
 * using a generalization of HTML called XML.  A typical XML fragment for a single news item will certainly
 * adhere to the format of the following example:
 *
 * <item>
 *   <title>At Installation Mass, New Pope Strikes a Tone of Openness</title>
 *   <link>http://www.nytimes.com/2005/04/24/international/worldspecial2/24cnd-pope.html</link>
 *   <description>The Mass, which drew 350,000 spectators, marked an important moment in the transformation of Benedict XVI.</description>
 *   <author>By IAN FISHER and LAURIE GOODSTEIN</author>
 *   <pubDate>Sun, 24 Apr 2005 00:00:00 EDT</pubDate>
 *   <guid isPermaLink="false">http://www.nytimes.com/2005/04/24/international/worldspecial2/24cnd-pope.html</guid>
 * </item>
 *
 * PullAllNewsItems reads and discards all characters up through the opening <item> tag (discarding the <item> tag
 * as well, because once it's read and indentified, it's been pulled,) and then hands the state of the stream to
 * ProcessSingleNewsItem, which handles the job of pulling and analyzing everything up through and including the </item>
 * tag. PullAllNewsItems processes the entire RSS feed and repeatedly advancing to the next <item> tag and then allowing
 * ProcessSingleNewsItem do process everything up until </item>.
 */

static const char *const kTextDelimiters = " \t\n\r\b!@$%^*()_+={[}]|\\'\":;/?.>,<~`";
static void PullAllNewsItems(urlconnection *urlconn, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  streamtokenizer st;
  STNew(&st, urlconn->dataStream, kTextDelimiters, false);
  while (GetNextItemTag(&st)) { // if true is returned, then assume that <item ...> has just been read and pulled from the data stream
    ProcessSingleNewsItem(&st, stopWords, indexedWords, indexedArticles);
  }
  
  STDispose(&st);
}

/**
 * Function: GetNextItemTag
 * ------------------------
 * Works more or less like GetNextTag below, but this time
 * we're searching for an <item> tag, since that marks the
 * beginning of a block of HTML that's relevant to us.  
 * 
 * Note that each tag is compared to "<item" and not "<item>".
 * That's because the item tag, though unlikely, could include
 * attributes and perhaps look like any one of these:
 *
 *   <item>
 *   <item rdf:about="Latin America reacts to the Vatican">
 *   <item requiresPassword=true>
 *
 * We're just trying to be as general as possible without
 * going overboard.  (Note that we use strncasecmp so that
 * string comparisons are case-insensitive.  That's the case
 * throughout the entire code base.)
 */

static const char *const kItemTagPrefix = "<item";
static bool GetNextItemTag(streamtokenizer *st)
{
  char htmlTag[1024];
  while (GetNextTag(st, htmlTag, sizeof(htmlTag))) {
    if (strncasecmp(htmlTag, kItemTagPrefix, strlen(kItemTagPrefix)) == 0) {
      return true;
    }
  }	 
  return false;
}

/**
 * Function: ProcessSingleNewsItem
 * -------------------------------
 * Code which parses the contents of a single <item> node within an RSS/XML feed.
 * At the moment this function is called, we're to assume that the <item> tag was just
 * read and that the streamtokenizer is currently pointing to everything else, as with:
 *   
 *      <title>Carrie Underwood takes American Idol Crown</title>
 *      <description>Oklahoma farm girl beats out Alabama rocker Bo Bice and 100,000 other contestants to win competition.</description>
 *      <link>http://www.nytimes.com/frontpagenews/2841028302.html</link>
 *   </item>
 *
 * ProcessSingleNewsItem parses everything up through and including the </item>, storing the title, link, and article
 * description in local buffers long enough so that the online new article identified by the link can itself be parsed
 * and indexed.  We don't rely on <title>, <link>, and <description> coming in any particular order.  We do asssume that
 * the link field exists (although we can certainly proceed if the title and article descrption are missing.)  There
 * are often other tags inside an item, but we ignore them.
 */

static const char *const kItemEndTag = "</item>";
static const char *const kTitleTagPrefix = "<title";
static const char *const kDescriptionTagPrefix = "<description";
static const char *const kLinkTagPrefix = "<link";
static void ProcessSingleNewsItem(streamtokenizer *st, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  char htmlTag[1024];
  char articleTitle[1024];
  char articleDescription[1024];
  char articleURL[1024];
  articleTitle[0] = articleDescription[0] = articleURL[0] = '\0';
  
  while (GetNextTag(st, htmlTag, sizeof(htmlTag)) && (strcasecmp(htmlTag, kItemEndTag) != 0)) {
    if (strncasecmp(htmlTag, kTitleTagPrefix, strlen(kTitleTagPrefix)) == 0) ExtractElement(st, htmlTag, articleTitle, sizeof(articleTitle));
    if (strncasecmp(htmlTag, kDescriptionTagPrefix, strlen(kDescriptionTagPrefix)) == 0) ExtractElement(st, htmlTag, articleDescription, sizeof(articleDescription));
    if (strncasecmp(htmlTag, kLinkTagPrefix, strlen(kLinkTagPrefix)) == 0) ExtractElement(st, htmlTag, articleURL, sizeof(articleURL));
  }
  
  if (strncmp(articleURL, "", sizeof(articleURL)) == 0) return;     // punt, since it's not going to take us anywhere
  ParseArticle(articleTitle, articleDescription, articleURL, stopWords, indexedWords, indexedArticles);
}

/**
 * Function: ExtractElement
 * ------------------------
 * Potentially pulls text from the stream up through and including the matching end tag.  It assumes that
 * the most recently extracted HTML tag resides in the buffer addressed by htmlTag.  The implementation
 * populates the specified data buffer with all of the text up to but not including the opening '<' of the
 * closing tag, and then skips over all of the closing tag as irrelevant.  Assuming for illustration purposes
 * that htmlTag addresses a buffer containing "<description" followed by other text, these three scanarios are
 * handled:
 *
 *    Normal Situation:     <description>http://some.server.com/someRelativePath.html</description>
 *    Uncommon Situation:   <description></description>
 *    Uncommon Situation:   <description/>
 *
 * In each of the second and third scenarios, the document has omitted the data.  This is not uncommon
 * for the description data to be missing, so we need to cover all three scenarious (I've actually seen all three.)
 * It would be quite unusual for the title and/or link fields to be empty, but this handles those possibilities too.
 */
 
static void ExtractElement(streamtokenizer *st, const char *htmlTag, char dataBuffer[], int bufferLength)
{
  assert(htmlTag[strlen(htmlTag) - 1] == '>');
  if (htmlTag[strlen(htmlTag) - 2] == '/') return;    // e.g. <description/> would state that a description is not being supplied
  STNextTokenUsingDifferentDelimiters(st, dataBuffer, bufferLength, "<");
  RemoveEscapeCharacters(dataBuffer);
  if (dataBuffer[0] == '<') strcpy(dataBuffer, "");  // e.g. <description></description> also means there's no description
  STSkipUntil(st, ">");
  STSkipOver(st, ">");
}

/** 
 * Function: ParseArticle
 * ----------------------
 * Attempts to establish a network connect to the news article identified by the three
 * parameters.  The network connection is either established of not.  The implementation
 * is prepared to handle a subset of possible (but by far the most common) scenarios,
 * and those scenarios are categorized by response code:
 *
 *    0 means that the server in the URL doesn't even exist or couldn't be contacted.
 *    200 means that the document exists and that a connection to that very document has
 *        been established.
 *    301 means that the document has moved to a new location
 *    302 also means that the document has moved to a new location
 *    4xx and 5xx (which are covered by the default case) means that either
 *        we didn't have access to the document (403), the document didn't exist (404),
 *        or that the server failed in some undocumented way (5xx).
 *
 * The are other response codes, but for the time being we're punting on them, since
 * no others appears all that often, and it'd be tedious to be fully exhaustive in our
 * enumeration of all possibilities.
 */

static void ParseArticle(const char *articleTitle, const char *articleDescription, const char *articleURL, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  url u;
  urlconnection urlconn;
  streamtokenizer st;

  URLNewAbsolute(&u, articleURL);
  URLConnectionNew(&urlconn, &u);
  
  switch (urlconn.responseCode) {
      case 0: printf("Unable to connect to \"%s\".  Domain name or IP address is nonexistent.\n", articleURL);
	      break;
      case 200: printf("Scanning \"%s\" from \"http://%s\"\n", articleTitle, u.serverName);
	        STNew(&st, urlconn.dataStream, kTextDelimiters, false);
		ScanArticle(&st, articleTitle, articleDescription, articleURL, stopWords, indexedWords, indexedArticles);
		STDispose(&st);
		break;
      case 301:
      case 302: // just pretend we have the redirected URL all along, though index using the new URL and not the old one...
	ParseArticle(articleTitle, articleDescription, urlconn.newUrl, stopWords, indexedWords, indexedArticles);
		break;
      default: printf("Unable to pull \"%s\" from \"%s\". [Response code: %d] Punting...\n", articleTitle, u.serverName, urlconn.responseCode);
	       break;
  }
  
  URLConnectionDispose(&urlconn);
  URLDispose(&u);
}

/**
 * Function: ScanArticle
 * ---------------------
 * Parses the specified article, skipping over all HTML tags, and counts the numbers
 * of well-formed words that could potentially serve as keys in the set of indices.
 * Once the full article has been scanned, the number of well-formed words is
 * printed, and the longest well-formed word we encountered along the way
 * is printed as well.
 *
 * This is really a placeholder implementation for what will ultimately be
 * code that indexes the specified content.
 */

static void ScanArticle(streamtokenizer *st, const char *articleTitle, const char *unused, const char *articleURL, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  int numWords = 0;
  char word[1024];
  char longestWord[1024] = {'\0'};

  while (STNextToken(st, word, sizeof(word))) {
    if (strcasecmp(word, "<") == 0) {
      SkipIrrelevantContent(st); // in html-utls.h
    } else {
      RemoveEscapeCharacters(word);
      char *dummy = word;
      if (WordIsWellFormed(word) && HashSetLookup(stopWords, &dummy) == NULL) {
	numWords++;
	if (strlen(word) > strlen(longestWord))
	  strcpy(longestWord, word);

	struct index idx;
	idx.word = strdup(word);
	
	struct index *existingIndex = HashSetLookup(indexedWords, &idx);
	if (existingIndex == NULL) {
	  idx.articles = malloc(sizeof(vector));
	  VectorNew(idx.articles, sizeof(struct article*), ArticleCountFree, 0);
	  
	  struct articleCount *artC = malloc(sizeof(struct articleCount));
	  artC->count = 1;

	  struct articleInfo *artI = malloc(sizeof(struct articleInfo));
	  artI->articleTitle = strdup(articleTitle);
	  artI->articleURL = strdup(articleURL);
	  
	  struct articleInfo *existingArtI = HashSetLookup(indexedArticles, &artI);
	  if (existingArtI == NULL) {
	    HashSetEnter(indexedArticles, &artI);
	    existingArtI = HashSetLookup(indexedArticles, &artI);
	  } else {
	    ArticleInfoFree(&artI);
	  }
	  artC->info = *(struct articleInfo **)existingArtI;

	  printf("Indexing 0x%08lx \"%s\" \"%s\" \"%s\"\n", (long unsigned int)artC, word, artC->info->articleTitle, artC->info->articleURL);
	  
	  VectorAppend(idx.articles, &artC);
	  
	  HashSetEnter(indexedWords, &idx);
	} else {
	  
	  free(idx.word); 
	}
      }
    }
  }

  printf("\tWe counted %d well-formed words [including duplicates].\n", numWords);
  printf("\tThe longest word scanned was \"%s\".", longestWord);
  if (strlen(longestWord) >= 15 && (strchr(longestWord, '-') == NULL)) 
    printf(" [Ooooo... long word!]");
  printf("\n");
}

/** 
 * Function: QueryIndices
 * ----------------------
 * Standard query loop that allows the user to specify a single search term, and
 * then proceeds (via ProcessResponse) to list up to 10 articles (sorted by relevance)
 * that contain that word.
 */

static void QueryIndices(hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  char response[1024];
  while (true) {
    printf("Please enter a single query term that might be in our set of indices [enter to quit]: ");
    fgets(response, sizeof(response), stdin);
    response[strlen(response) - 1] = '\0';
    if (strcasecmp(response, "") == 0) break;
    ProcessResponse(response, stopWords, indexedWords, indexedArticles);
  }
}

/** 
 * Function: ProcessResponse
 * -------------------------
 * Placeholder implementation for what will become the search of a set of indices
 * for a list of web documents containing the specified word.
 */

static void ProcessResponse(const char *word, hashset *stopWords, hashset *indexedWords, hashset *indexedArticles)
{
  if (!WordIsWellFormed(word)) {
    printf("\tWe won't be allowing words like \"%s\" into our set of indices.\n", word);
  } else if (HashSetLookup(stopWords, &word) != NULL) {
    printf("\t\"%s\" is too common a word to be taken seriously.  Please be more specific.\n", word);
  } else {
    struct index *idx = HashSetLookup(indexedWords, &word);
    if (idx != NULL) {
      vector *v = idx->articles;
      int n = VectorLength(v);
      printf("Nice! We found %d articles that include the word \"%s\".\n\n", n, idx->word);
      
      for (int i = 0; i < n; i++) {
	struct articleCount *artC = *(struct articleCount **)VectorNth(v, i);
	struct articleInfo *artI = artC->info;
	printf("\t%d.) \"%s\" [search term occurs %d times]\n", i+1, artI->articleTitle, artC->count);
	printf("\t     \"%s\"\n", artI->articleURL);
      }
      printf("\n");
    } else {
      printf("\tNone of today's news articles contain the word \"%s\".\n", word);
    }
  }
}

/**
 * Predicate Function: WordIsWellFormed
 * ------------------------------------
 * Before we allow a word to be inserted into our map
 * of indices, we'd like to confirm that it's a good search term.
 * One could generalize this function to allow different criteria, but
 * this version hard codes the requirement that a word begin with 
 * a letter of the alphabet and that all letters are either letters, numbers,
 * or the '-' character.  
 */

static bool WordIsWellFormed(const char *word)
{
  int i;
  if (strlen(word) == 0) return true;
  if (!isalpha((int) word[0])) return false;
  for (i = 1; i < strlen(word); i++)
    if (!isalnum((int) word[i]) && (word[i] != '-')) return false; 

  return true;
}
