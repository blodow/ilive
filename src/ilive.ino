#include <stdlib.h>
#include <math.h>

bool match(const char* buf, int pos, const char* tag, const char* tagEnd) {
    return (buf[pos] == '<' && strncmp(buf+pos+1, tag, tagEnd-tag) == 0);
}

int getTagNameEnd(const char* buf, int start) {
    while (buf[start] != 0 && buf[start] != ' ' && buf[start] != '>') {
        ++start;
    }
    return start;
}

int eatOpenTag(const char* buf, int pos) {
    while (buf[pos] != 0 && buf[pos] != '>') {
        ++pos;
    }
    if (buf[pos] != 0) {
        ++pos;
    }
    return pos;
}

int eatCloseTag(const char* buf, int pos) {
    const char* tag = buf+pos+1;
    pos = getTagNameEnd(buf, pos);
    const char* tagEnd = buf+pos;
    // For now, simply look for any matching closing tag.
    while (buf[pos] != 0) {
        if (buf[pos] == '<' && buf[pos+1] == '/' && buf[pos+2+(tagEnd-tag)] == '>' && strncmp(buf+pos+2, tag, tagEnd-tag) == 0) {
          return pos+3+tagEnd-tag;
        }
        ++pos;
    }
    return pos;
}

int eatText(const char *buf, int pos, const char* tag, const char* tagEnd) {
    while (buf[pos] != 0) {
        if (buf[pos] == '<' && buf[pos+1] == '/' && buf[pos+2+(tagEnd-tag)] == '>' && strncmp(buf+pos+2, tag, tagEnd-tag) == 0) {
          return pos;
        }
        ++pos;
    }
    return pos;
}


bool getText(const char* buf, const char *path[], int path_items, const char** text, const char** textEnd) {
  int bufLen = strlen(buf);
  int path_pos = 0;
  int pos = 0;

  while(pos < bufLen) {
      const char *tag = path[path_pos];
      const char* tagEnd = tag + strlen(path[path_pos]);
      if (match(buf, pos, tag, tagEnd)) {
          pos = eatOpenTag(buf, pos);
          if (buf[pos] != 0) {
            if (path_pos + 1 >= path_items) {
              int end = eatText(buf, pos, tag, tagEnd);
              *text = buf+pos;
              *textEnd = buf+end;
              return true;
            } else {
              path_pos += 1;
            }
          } else {
            return false;
          }
      } else {
          pos = eatCloseTag(buf, pos);
      }
  }
  return false;
}

long strtola (const char* start, const char* end) {
    char lenStr[1+end-start];
    std::copy(start, end, lenStr);
    lenStr[end-start] = 0;
    return strtol(lenStr, NULL, 10);
}

int getAmp() {
  const char *resp = "<YAMAHA_AV rsp=\"GET\" RC=\"0\"><foo></foo><Main_Zone><Volume><Lvl><Val>-445</Val><Exp>1</Exp><Unit>dB</Unit></Lvl></Volume></Main_Zone></YAMAHA_AV>";
  int resp_len = strlen(resp);
  const char *pathVol[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Val"};
  const char *pathUnit[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Unit"};
  const char *pathExp[] = {"YAMAHA_AV", "Main_Zone", "Volume", "Lvl", "Exp"};
  int path_items = 5;
  int path_pos = 0;
  int pos = 0;
  const char* text = NULL;
  const char* textEnd = NULL;

  char* unit = NULL;

  float vol = 0;
  if (getText(resp, pathVol, 5, &text, &textEnd)) {
    vol = strtola(text, textEnd);
    std::cerr << "found vol: '";
    std::copy(text, textEnd, std::ostream_iterator<char>(std::cout));
    std::cout << "' = " << vol << std::endl;
  }
  if (getText(resp, pathUnit, 5, &text, &textEnd)) {
    unit = (char*)calloc(sizeof(char), 1+textEnd-text);
    std::copy(text, textEnd, unit);
    std::cerr << "found unit: '";
    std::copy(text, textEnd, std::ostream_iterator<char>(std::cout));
    std::cout << "'" << std::endl;
  }
  if (getText(resp, pathExp, 5, &text, &textEnd)) {
    vol /= pow(10, strtola(text, textEnd));
    std::cerr << "found exp: '";
    std::copy(text, textEnd, std::ostream_iterator<char>(std::cout));
    std::cout << "'" << std::endl;
  }

  std::cerr << "Volume = " << vol << " " << unit << std::endl;
  
  return vol;
}

