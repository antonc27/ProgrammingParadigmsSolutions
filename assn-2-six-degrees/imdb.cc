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

void imdb::checkSizes() const
{
    assert(sizeof(char) == 1);
    assert(sizeof(short) == 2);
    assert(sizeof(int) == 4);
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

const char *movieNameFromOffset(const void *base, const void *offset)
{
    int movieOffset = *(int *)offset;
    return (char *)base + movieOffset;
}

void fillMovieFromOffset(film *movie, const void *base, const void *offset)
{
  const char *movieName = movieNameFromOffset(base, offset);

  size_t movieNameLen = strlen(movieName) + 1;
  int year = *(movieName + movieNameLen) + 1900;

  movie->title = movieName;
  movie->year = year;
}

void *findActor(const string& player, const void *base)
{
  int totalSize = *(int *)base;
  void *firstActor = (int *)base + 1;
  
  actorWrap wrap;
  wrap.actorName = player.c_str();
  wrap.actors = base;

  return bsearch(&wrap, firstActor, totalSize, sizeof(int), actorCmp);
}

bool imdb::getCredits(const string& player, vector<film>& films) const
{
  checkSizes();
    
  void *found = findActor(player, actorFile);

  if (found != NULL) {
    const char *actorName = actorNameFromOffset(actorFile, found);
      
    size_t actorNameLen = strlen(actorName) + 1;
    int actorNamePadding = (actorNameLen % 2 == 0) ? 0 : 1;
    short moviesSize = *(short *)((char *)actorName + actorNameLen + actorNamePadding);
    
    size_t moviesSizeLen = actorNameLen + actorNamePadding + 2;
    int moviesSizePadding = (moviesSizeLen % 4 == 0) ? 0 : 2;
      
    int *startOfMoviesArr = (int *)((char *)actorName + moviesSizeLen + moviesSizePadding);
    for (int i = 0; i < moviesSize; i++) {
      film movie;
      fillMovieFromOffset(&movie, movieFile, startOfMoviesArr + i);
      films.push_back(movie);
    }
  }
  
  return (found != NULL);
}

struct movieWrap {
  const film *movie;
  const void *movies;
};

int movieCmp(const void *first, const void *second)
{
  film movie = *((movieWrap *)first)->movie;
  const void *movies = ((movieWrap *)first)->movies;

  film anotherMovie;
  fillMovieFromOffset(&anotherMovie, movies, second);

  if (movie == anotherMovie) {
    return 0;
  } else if (movie < anotherMovie) {
    return -1;
  } else {
    return 1;
  }
}

void *findMovie(const film& movie, const void *base)
{
  int totalSize = *(int *)base;
  void *firstMovie = (int *)base + 1;
  
  movieWrap wrap;
  wrap.movie = &movie;
  wrap.movies = base;

  return bsearch(&wrap, firstMovie, totalSize, sizeof(int), movieCmp);
}

bool imdb::getCast(const film& movie, vector<string>& players) const
{
  checkSizes();
    
  void *found = findMovie(movie, movieFile);

  if (found != NULL) {
    const char *movieName = movieNameFromOffset(movieFile, found);
    size_t movieNameLen = strlen(movieName) + 1;
        
    int movieYearPadding = ((movieNameLen + 1) % 2 == 0) ? 0 : 1;
    short actorsSize = *(short *)(movieName + movieNameLen + 1 + movieYearPadding);
    
    size_t actorsSizeLen = movieNameLen + 1 + movieYearPadding + 2;
    int actorsSizePadding = (actorsSizeLen % 4 == 0) ? 0 : 2;

    int *startOfActorsArr = (int *)((char *)movieName + actorsSizeLen + actorsSizePadding);
    for (int i = 0; i < actorsSize; i++) {
      const char *actorName = actorNameFromOffset(actorFile, startOfActorsArr + i);
      players.push_back(actorName);
    }
  }
  
  return (found != NULL);
}

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
