C4_TYPE = 0

EXE_ZEM5007131 := zem500

EXECUTABLE := main

ARCH:=mips
CROSS_COMPILE_ARCA := arca-
CROSS_COMPILE_MIPS := mipsel-
CROSS_COMPILE := $(CROSS_COMPILE_MIPS)
#CROSS_COMPILE := $(CROSS_COMPILE_ARCA)
ZEM300 := -DZEM300 
#WEBLIB :=  ./webinterface/libdl.a

ifeq ($(C4_TYPE),1)
MACHINETYPE := -DMACHINETYPE_C4
endif


###ZEM500 MUST DEFINE FOLLOWING###
BOARD_ZEM500 := -DZEM500



PLATFORM := ../platform
PLATFORMZEM := ../platform/$(MAKECMDGOALS)
PUBLIC := ../public
FINGERPRINT := ../fingerprint
LCD := ../lcd
DB := ../db
COMM := ../comm
COMM_NET := ../comm/net
COMM_RS232 := ../comm/rs232

MATHLIB := -lm
MATHLIB := $(PLATFORMZEM)/libm.a
CHANGEMACLIB = $(PLATFORMZEM)/libnandflashstatic.a

CFLAGS := -O2 -I. -I./$(MAKECMDGOALS) -I$(PLATFORM) -I$(PLATFORMZEM) -I$(PUBLIC) -I$(FINGERPRINT) -I$(LCD) -I$(DB)  -Wall -D__GNU__  $(PUSH_SDK) $(HTTPD_SDK)
$(EXE_ZEM5007131):CFLAGS += $(ZEM300) $(BOARD_ZEM500) $(TTS_SUPPORT) $(PROCESS) $(MACHINETYPE)

CXXFLAGS := $(CFLAGS)
CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip

FPLIB := -L./$(PLATFORMZEM) -lzkfp -ldlcl


SOURCE := $(wildcard *.c) $(wildcard *.cc) $(wildcard $(MAKECMDGOALS)/*.c $(PLATFORM)/*.c $(PUBLIC)/*.c $(FINGERPRINT)/*.c $(LCD)/*.c $(DB)/*.c $(PLATFORMZEM)/*.c $(COMM_NET)/*.c $(COMM_RS232)/*.c)
OBJS := $(patsubst %.c,%.o,$(patsubst %.cc,%.o, $(SOURCE)))
DEPS := $(patsubst %.o,%.d,$(OBJS))
MISSING_DEPS := $(filter-out $(wildcard $(DEPS)),$(DEPS))
MISSING_DEPS_SOURCES := $(wildcard $(patsubst %.d,%.c,$(MISSING_DEPS)) \
$(patsubst %.d,%.cc,$(MISSING_DEPS)))
CPPFLAGS += -MD

$(EXE_ZEM5007131) : $(OBJS)
	$(CROSS_COMPILE)gcc -o $(EXECUTABLE) $(OBJS) $(FPLIB) $(CHANGEMACLIB)  -lcrypt -L$(MATHLIB)
	$(STRIP) --strip-all $(EXECUTABLE)


ifeq ($(C4_TYPE),1)
	cp $(EXECUTABLE)	 /var/tftpboot/comm_c4_$(LOGNAME)
else
	cp $(EXECUTABLE)	 /var/tftpboot/comm_$(LOGNAME)
endif

clean:
	rm  -f $(OBJS) 
	rm -f ./$(EXE_ZEM5007131)/*.o
	rm -f ./$(EXE_ARCA)/*.o
	rm -f ./$(EXE_LUMIDIGM)/*.o
	rm -f ./$(EXE_IRSENSOR)/*.o
	rm -f ./$(EXE_IRSENSOR)/*.d
	
	rm -f ./$(PLATFORM)/*.o
	rm -f ./$(PLATFORM)/*.d
	rm -f ./$(PLATFORM)/zem500/*.o
	rm -f ./$(PLATFORM)/zem500/*.d
	rm -f ./$(PUBLIC)/*.o
	rm -f ./$(PUBLIC)/*.d
	rm -f ./$(FINGERPRINT)/*.o
	rm -f ./$(FINGERPRINT)/*.d
	rm -f ./$(LCD)/*.o
	rm -f ./$(LCD)/*.d
	rm -f ./$(DB)/*.o
	rm -f ./$(DB)/*.d
	rm -f ./$(COMM)/*.o
	rm -f ./$(COMM)/*.d
	rm -f ./$(COMM_NET)/*.o
	rm -f ./$(COMM_NET)/*.d
	rm -f ./$(COMM_RS232)/*.o
	rm -f ./$(COMM_RS232)/*.d
	rm -f *.d
	rm -f main.gz
	rm -f main.tgz
cleanall:
	rm  -f $(OBJS)
	rm  *.h~
	rm  *.c~
	rm  *.d

