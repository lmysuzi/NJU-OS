#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <math.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

// Copied from the manual
struct fat32hdr {
  u8  BS_jmpBoot[3];
  u8  BS_OEMName[8];
  u16 BPB_BytsPerSec;
  u8  BPB_SecPerClus;
  u16 BPB_RsvdSecCnt;
  u8  BPB_NumFATs;
  u16 BPB_RootEntCnt;
  u16 BPB_TotSec16;
  u8  BPB_Media;
  u16 BPB_FATSz16;
  u16 BPB_SecPerTrk;
  u16 BPB_NumHeads;
  u32 BPB_HiddSec;
  u32 BPB_TotSec32;
  u32 BPB_FATSz32;
  u16 BPB_ExtFlags;
  u16 BPB_FSVer;
  u32 BPB_RootClus;
  u16 BPB_FSInfo;
  u16 BPB_BkBootSec;
  u8  BPB_Reserved[12];
  u8  BS_DrvNum;
  u8  BS_Reserved1;
  u8  BS_BootSig;
  u32 BS_VolID;
  u8  BS_VolLab[11];
  u8  BS_FilSysType[8];
  u8  __padding_1[420];
  u16 Signature_word;
} __attribute__((packed));

struct fat32dent {
  u8  DIR_Name[11];
  u8  DIR_Attr;
  u8  DIR_NTRes;
  u8  DIR_CrtTimeTenth;
  u16 DIR_CrtTime;
  u16 DIR_CrtDate;
  u16 DIR_LastAccDate;
  u16 DIR_FstClusHI;
  u16 DIR_WrtTime;
  u16 DIR_WrtDate;
  u16 DIR_FstClusLO;
  u32 DIR_FileSize;
} __attribute__((packed));

struct fat32longdent{
    u8  LDIR_Ord;
    u8  LDIR_Name1[10];
    u8  LDIR_Attr;
    u8  LDIR_Type;
    u8  LDIR_Chksum;
    u8  LDIR_Name2[12];
    u16 LDIR_FstClusLO;
    u8  LDIR_Name3[4];
}__attribute__((packed));

struct bmp_t {
  char id[2];
  u32 size;
  u32 res;
  u32 offset;
}__attribute__((packed));

typedef struct fat32dent SDIR;
typedef struct fat32longdent LDIR;
typedef struct bmp_t bmp_t;

typedef union{
  SDIR sdir;
  LDIR ldir;
}DIR;


#define LAST_LONG_ENTRY         (0x40)
#define ATTR_READ_ONLY          (0x01)
#define ATTR_HIDDEN             (0x02)
#define ATTR_SYSTEM             (0x04)
#define ATTR_VOLUME_ID          (0x08)
#define ATTR_DIRECTORY          (0x10)
#define ATTR_ARCHIVE            (0x20)
#define ATTR_LONG_NAME          (ATTR_READ_ONLY | ATTR_HIDDEN | ATTR_SYSTEM | ATTR_VOLUME_ID)
#define ATTR_NUL                (0x0)
#define ATTR_DIR                (0x1)
#define ATTR_FIL                (0x2)

struct fat32hdr *hdr;
u8 *data_region_addr;
u8 *end_addr;
size_t bytes_per_clus;
size_t file_size;
size_t entry_size;
u8 count=0;

void get_filename(struct fat32dent *dent, char *buf) {
  // RTFM: Sec 6.1
  int len = 0;
  for (int i = 0; i < sizeof(dent->DIR_Name); i++) {
    if (dent->DIR_Name[i] != ' ') {
      if (i == 8) buf[len++] = '.';
      buf[len++] = dent->DIR_Name[i];
    }
  }
  buf[len] = '\0';
}

void *map_disk(const char *fname);



int is_dir(DIR *dir){
  if(dir->sdir.DIR_Name[0] == 0)return 0;//未使用过

  for(int i=0;i<entry_size;){

    if(dir[i].sdir.DIR_Name[0]=='\xe5')i++; //无效
    else if(dir[i].ldir.LDIR_Attr==ATTR_LONG_NAME) {
      
      size_t size = dir[i].ldir.LDIR_Ord;
      if(dir[i].ldir.LDIR_Ord>LAST_LONG_ENTRY){size^=LAST_LONG_ENTRY;}

      size=size<=(entry_size-1-i)?size:(entry_size-1-i);

      if(dir[i].ldir.LDIR_Type!=0||dir[i].ldir.LDIR_FstClusLO!=0)return 0;
      for(int j=1;j<size;j++){
        if(dir[i+j].ldir.LDIR_Ord!=(size-j)||dir[i+j].ldir.LDIR_Type!=0||dir[i+j].ldir.LDIR_FstClusLO!=0){return 0;}
      }

      if(i+size<entry_size&&(dir[size].sdir.DIR_NTRes!=0||dir[size].sdir.DIR_Name[0]==0)){return 0;}

      i+=size+1;

      }
      else if(dir[i].sdir.DIR_Name[0]){
        if(dir[i].sdir.DIR_NTRes!=0||dir[i].sdir.DIR_Name[0]==0){return 0;}
        i++;
      }
      else{
        u8 *begin = (uint8_t*)(&dir[i]), *end = (uint8_t*)(&dir[entry_size]);
        for(u8 *iter=begin;iter<end;++iter){
          if(*iter){return 0;}
        }
        break;
      }
    }

    return 1;
}

int inline isbmp(bmp_t *bmp,int size){
  if(bmp->id[0]!='B'||bmp->id[1]!='M')return 0;
  if(bmp->size!=size)return 0;
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
    exit(1);
  }

  setbuf(stdout, NULL);

  assert(sizeof(struct fat32hdr) == 512); // defensive

  // map disk image to memory
  hdr = map_disk(argv[1]);
  printf("%d\n",hdr->BPB_RootClus);

  data_region_addr=((u8 *)hdr+(hdr->BPB_RsvdSecCnt+hdr->BPB_NumFATs*hdr->BPB_FATSz32)*hdr->BPB_BytsPerSec);
  end_addr=((u8*)hdr+file_size);
  bytes_per_clus=hdr->BPB_BytsPerSec*hdr->BPB_SecPerClus;
  entry_size = bytes_per_clus/sizeof(DIR);

  for(u8 *addr=data_region_addr;addr<end_addr;addr+=bytes_per_clus){
    DIR *clus=(DIR *)addr;
    if(is_dir(clus)){
       int ndents = hdr->BPB_BytsPerSec * hdr->BPB_SecPerClus / sizeof(struct fat32dent);

      for (int d = 0; d < ndents; d++) {
        struct fat32dent *dent = (struct fat32dent *)clus + d;
        if (dent->DIR_Name[0] == 0x00 ||
            dent->DIR_Name[0] == 0xe5 ||
            dent->DIR_Attr & ATTR_HIDDEN) continue;

        char fname[32];
        get_filename(dent, fname);
        printf("%s\n",fname);
        u32 Clusid = dent->DIR_FstClusLO | (dent->DIR_FstClusHI << 16);
        u8 *addr=data_region_addr+(Clusid-hdr->BPB_RootClus)*bytes_per_clus;
        bmp_t *bmp=(bmp_t *)addr;
        if(!isbmp(bmp,dent->DIR_FileSize))continue;
        char path[40]="../../Pictures/";
        char str[3];
        sprintf(str,"%d",count++);
        strcat(path,str);
        printf("%s\n",path);

      } 
        
    }
  }
  // TODO: frecov

  // file system traversal
  munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
}

void *map_disk(const char *fname) {
  int fd = open(fname, O_RDWR);

  if (fd < 0) {
    perror(fname);
    goto release;
  }

  off_t size = lseek(fd, 0, SEEK_END);
  if (size == -1) {
    perror(fname);
    goto release;
  }

  struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

  file_size=size;
  

  if (hdr == (void *)-1) {
    goto release;
  }

  close(fd);

  if (hdr->Signature_word != 0xaa55 ||
      hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec != size) {
    fprintf(stderr, "%s: Not a FAT file image\n", fname);
    goto release;
  }
  return hdr;

release:
  if (fd > 0) {
    close(fd);
  }
  exit(1);
}
