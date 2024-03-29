/*
 * logo.c: 'EnigmaNG' skin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 */

#include "common.h"
#include "bitmap.h"
#include "config.h"
#include "logo.h"

#include <vdr/plugin.h>

cEnigmaLogoCache EnigmaLogoCache(0);

cEnigmaLogoCache::cEnigmaLogoCache(unsigned int cacheSizeP) :cacheSizeM(cacheSizeP), bitmapM(NULL)
{
#ifdef HAVE_IMAGEMAGICK
  bmpImage = new cBitmap(8, 8, 1);
  cOSDImageBitmap::Init();
#endif
}

cEnigmaLogoCache::~cEnigmaLogoCache()
{
#ifdef HAVE_IMAGEMAGICK
  delete bmpImage;
#endif

  // let's flush the cache
  Flush();
}

bool cEnigmaLogoCache::Resize(unsigned int cacheSizeP)
{
  debug("cPluginSkinEnigma::Resize(%d)", cacheSizeP);
  // flush cache only if it's smaller than before
  if (cacheSizeP < cacheSizeM) {
    Flush();
  }
  // resize current cache
  cacheSizeM = cacheSizeP;
  return true;
}

bool cEnigmaLogoCache::DrawEventImage(const cEvent *Event, int x, int y, int w, int h, int c, cOsd *osd)
{
  if (Event == NULL || osd == NULL)
    return false;

  char *strFilename = NULL;
  int rc = false;
  if (-1 != asprintf(&strFilename, "%s/%d_0.%s", EnigmaConfig.GetImagesDir(), Event->EventID(), EnigmaConfig.GetImageExtension())) {
    rc = DrawImage(strFilename, x, y, w, h, c, osd);
    free (strFilename);
  }
  return rc;
}

bool cEnigmaLogoCache::DrawRecordingImage(const cRecording *Recording, int x, int y, int w, int h, int c, cOsd *osd)
{
  if (Recording == NULL || osd == NULL)
    return false;

  char *strFilename = NULL;
  int rc = false;
  if (-1 != asprintf(&strFilename, "%s/%s_0.%s", Recording->FileName(), RECORDING_COVER, EnigmaConfig.GetImageExtension())) {
    rc = DrawImage(strFilename, x, y, w, h, c, osd);
    free (strFilename);
  }
  return rc;
}

bool cEnigmaLogoCache::DrawImage(const char *fileNameP, int x, int y, int w, int h, int c, cOsd *osd)
{
  if (fileNameP== NULL || osd == NULL)
    return false;

  struct stat stbuf;
  if (lstat(fileNameP, &stbuf) != 0) {
    error("cPluginSkinEnigma::LoadImage(%s) FILE NOT FOUND", fileNameP);
    bitmapM = NULL;
    return false;
  }

#ifdef HAVE_IMAGEMAGICK
  bitmapM = NULL;
  return image.DrawImage(fileNameP, x, y, w, h, c, osd);
#else
  int rc = LoadXpm(fileNameP, w, h);
  if (rc)
    osd->DrawBitmap(x, y, *bitmapM); //TODO?
  return rc;
#endif
}

bool cEnigmaLogoCache::LoadChannelLogo(const cChannel *Channel)
{
  if (Channel == NULL)
    return false;

  bool fFoundLogo = false;
  const char *logoname = NULL;
  char *strLogo = NULL;
  char *filename = NULL;

  char *strChannelID = EnigmaConfig.useChannelId && !Channel->GroupSep() ? strdup(*Channel->GetChannelID().ToString()) : NULL;
  logoname = EnigmaConfig.useChannelId && !Channel->GroupSep() ? strChannelID : Channel->Name();
  if (logoname == NULL) goto leave;

  strLogo = strreplace(strdup(logoname), '/', '~');
  if (strLogo == NULL) goto leave;

  filename = (char *)malloc(strlen(strLogo) + 20 /* should be enough for folder */);
  if (filename == NULL) goto leave;

  strcpy(filename, "logos/");
  strcat(filename, strLogo);
  if (!(fFoundLogo = Load(filename, EnigmaConfig.channelLogoWidth, EnigmaConfig.channelLogoHeight))) {
    fFoundLogo = Load("logos/no_logo", EnigmaConfig.channelLogoWidth, EnigmaConfig.channelLogoHeight); //TODO? different default logo for channel/group?
  }

leave:
  free(filename);
  free(strLogo);
  free(strChannelID);

  return fFoundLogo;
}

bool cEnigmaLogoCache::LoadSymbol(const char *fileNameP)
{
  return Load(fileNameP, SymbolWidth, SymbolHeight);
}

bool cEnigmaLogoCache::LoadIcon(const char *fileNameP)
{
  return Load(fileNameP, IconWidth, IconHeight);
}

bool cEnigmaLogoCache::Load(const char *fileNameP, int w, int h)
{
  if (fileNameP == NULL)
    return false;

  char *strFilename = NULL;
  if (-1 == asprintf(&strFilename, "%s/%s.xpm", EnigmaConfig.GetLogoDir(), fileNameP))
    return false;

  debug("cPluginSkinEnigma::Load(%s)", strFilename);
  // does the logo exist already in map
  std::map < std::string, cBitmap * >::iterator i = cacheMapM.find(strFilename);
  if (i != cacheMapM.end()) {
    // yes - cache hit!
    debug("cPluginSkinEnigma::Load() CACHE HIT!");
    // check if logo really exist
    if (i->second == NULL) {
      debug("cPluginSkinEnigma::Load() EMPTY");
      // empty logo in cache
      free(strFilename);
      return false;
    }
    bitmapM = i->second;
  } else {
    // no - cache miss!
    debug("cPluginSkinEnigma::Load() CACHE MISS!");
    // try to load xpm logo
    if (!LoadXpm(strFilename, w, h)) {
      free(strFilename);
      return false;
    }
    // check if cache is active
    if (cacheSizeM) {
      // update map
      if (cacheMapM.size() >= cacheSizeM) {
        // cache full - remove first
        debug("cPluginSkinEnigma::Load() DELETE");
        if (cacheMapM.begin()->second != NULL) {
          // logo exists - delete it
          cBitmap *bmp = cacheMapM.begin()->second;
          DELETENULL(bmp);
        }
        // erase item
        cacheMapM.erase(cacheMapM.begin());
      }
      // insert logo into map
      debug("cPluginSkinEnigma::Load() INSERT(%s)", strFilename);
      cacheMapM.insert(std::make_pair(strFilename, bitmapM));
    }
    // check if logo really exist
    if (bitmapM == NULL) {
      debug("cPluginSkinEnigma::Load() EMPTY");
      // empty logo in cache
      free(strFilename);
      return false;
    }
  }
  free(strFilename);
  return true;
}

cBitmap & cEnigmaLogoCache::Get(void)
{
  return *bitmapM;
}

bool cEnigmaLogoCache::LoadXpm(const char *fileNameP, int w, int h)
{
  if (fileNameP == NULL)
    return false;

  cBitmap *bmp = new cBitmap(1, 1, 1);

  // create absolute filename
  debug("cPluginSkinEnigma::LoadXpm(%s)", fileNameP);
  // check validity
  if (access(fileNameP, R_OK) == 0) {
    if (bmp->LoadXpm(fileNameP)) {
      if ((bmp->Width() <= w) && (bmp->Height() <= h)) {
        debug("cPluginSkinEnigma::LoadXpm(%s) LOGO FOUND", fileNameP);
        // assign bitmap
        bitmapM = bmp;
        return true;
      } else {
        // wrong size
        error("cPluginSkinEnigma::LoadXpm(%s) LOGO HAS WRONG SIZE %d/%d (%d/%d)", fileNameP, bmp->Width(), bmp->Height(), w, h);
      }
    } else {
      // bmp->LoadXpm() failed
      error("cPluginSkinEnigma::LoadXpm(%s) VDR can't load logo", fileNameP);
    }
  } else {
    // no xpm logo found
    error("cPluginSkinEnigma::LoadXpm(%s) LOGO NOT FOUND/READABLE (%m)", fileNameP);
  }

  delete bmp;
  bitmapM = NULL;
  return false;
}

bool cEnigmaLogoCache::Flush(void)
{
  debug("cPluginSkinEnigma::Flush()");
  // check if map is empty
  if (!cacheMapM.empty()) {
    debug("cPluginSkinEnigma::Flush() NON-EMPTY");
    // delete bitmaps and clear map
    for (std::map<std::string, cBitmap *>::iterator i = cacheMapM.begin(); i != cacheMapM.end(); ++i) {
      delete((*i).second);
    }
    cacheMapM.clear();
    // nullify bitmap pointer
    bitmapM = NULL;
  }
  return true;
}

// vim:et:sw=2:ts=2:
