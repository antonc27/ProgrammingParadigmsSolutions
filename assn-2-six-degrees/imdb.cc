using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "imdb.h"

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";

imdb::imdb(const string& directory)
{
  const string actorFileName = directory + "/" + kActorFileName;
  const string movieFileName = directory + "/" + kMovieFileName;
  
  actorFile = acquireFileMap(actorFileName, actorInfo);
  movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const
{
  return !( (actorInfo.fd == -1) || 
	    (movieInfo.fd == -1) ); 
}

struct actorWrap {
  const char *actorName;
  const void *actors;
};

const char *actorNameFromOffset(const void *base, const void *offset)
{
  int realOffset = *(int *)offset;
  return ((char *)base + realOffset);
}

int actorCmp(const void *first, const void *second)
{
  const char *actorName = ((actorWrap *)first)->actorName;
  const void *actors = ((actorWrap *)first)->actors;

  const char *anotherActorName = actorNameFromOffset(actors, second);
  
  return strcmp(actorName, anotherActorName);
}

const movie *movieFromOffset(const void *base\
			     , const void *offset)
{
  int movieOffset = (int *)offset;//startOfMoviesArr[i];
  char *movieName = (char *)base + movi\
    eOffset;

  size_t movieNameLen = strlen(movieName) + \
    1;
  int year = *(movieName + movieNameLen) + 1	\
    900;

  film movie;
  movie.title = movieName;
  movie.year = year;
  return &movie;
}

bool imdb::getCredits(const string& player, vector<film>& films) const
{
  int totalSize = *(int *)actorFile;
  
  void *firstActor = (int *)actorFile + 1;
  
  actorWrap wrap;
  wrap.actorName = player.c_str();
  wrap.actors = actorFile;

  void *found = bsearch(&wrap, firstActor, totalSize, sizeof(int), actorCmp);

  if (found != NULL) {
    const char *actorName = actorNameFromOffset(actorFile, found);
      
    size_t actorNameLen = strlen(actorName) + 1;
    int actorNamePadding = (actorNameLen % 2 == 0) ? 0 : 1;
    short moviesSize = *(short *)((char *)actorName + actorNameLen + actorNamePadding);
      
    assert(sizeof(short) == 2);
    size_t moviesSizeLen = actorNameLen + actorNamePadding + 2;
    int moviesSizePadding = (moviesSizeLen % 4 == 0) ? 0 : 2;
      
    int *startOfMoviesArr = (int *)((char *)actorName + moviesSizeLen + moviesSizePadding);
    for (int i = 0; i < moviesSize; i++) {
      /*
      int movieOffset = startOfMoviesArr[i];
      char *movieName = (char *)movieFile + movieOffset;

      size_t movieNameLen = strlen(movieName) + 1;
      int year = *(movieName + movieNameLen) + 1900;

      film movie;
      movie.title = movieName;
      movie.year = year;
      */
      film mobie = 
      films.push_back(movie);
    }
  }
  
  return (found != NULL);
}

bool imdb::getCast(const film& movie, vector<string>& players) const { return false; }

imdb::~imdb()
{
  releaseFileMap(actorInfo);
  releaseFileMap(movieInfo);
}

// ignore everything below... it's all UNIXy stuff in place to make a file look like
// an array of bytes in RAM.. 
const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info)
{
  struct stat stats;
  stat(fileName.c_str(), &stats);
  info.fileSize = stats.st_size;
  info.fd = open(fileName.c_str(), O_RDONLY);
  return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info)
{
  if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
  if (info.fd != -1) close(info.fd);
}
