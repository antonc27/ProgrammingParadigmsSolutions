using namespace std;
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
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

int actorCmp(const void *first, const void *second)
{
  const char *actorName = ((actorWrap *)first)->actorName;
  const void *actors = ((actorWrap *)first)->actors;
  
  return 0;
}

bool imdb::getCredits(const string& player, vector<film>& films) const
{
  int totalSize = *(int *)actorFile;
  
  void *firstActor = (int *)actorFile + 1;
  
  actorWrap wrap;
  wrap.actorName = player.c_str();
  wrap.actors = actorFile;

  void *p = bsearch(&wrap, firstActor, totalSize, sizeof(int), actorCmp);
  
  return false;
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
