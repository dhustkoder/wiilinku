#-------------------------------------------------------------------------------
.SUFFIXES:
#-------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)

include $(DEVKITPRO)/wut/share/wut_rules

#-------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# DATA is a list of directories containing data files
# INCLUDES is a list of directories containing header files
#-------------------------------------------------------------------------------

#check WLU_VERSION_STR


GITTAG := $(shell git describe --tags --abbrev=0)

GITTAG_HASH := $(shell git rev-list -n 1 --abbrev-commit $(GITTAG))

GITHASH := $(shell git rev-parse --short HEAD)

ifeq ($(GITTAG_HASH), $(GITHASH))
	WLU_VERSION_STR:=$(GITTAG)
else
	WLU_VERSION_STR:=$(GITTAG)-$(GITHASH)
endif

$(shell git diff --quiet)
ifeq ($(.SHELLSTATUS), 1)
	WLU_VERSION_STR:=$(WLU_VERSION_STR)-dirty
endif

ifeq ($(WLU_VERSION_STR),)
	WLU_VERSION_STR:=unknown-version
endif


ifeq ($(BUILD_TYPE),Release)
OUTPUTDIR    := wiiu_release_build
else
WLU_VERSION_STR:=$(WLU_VERSION_STR)-debug
BUILD_TYPE   = Debug
OUTPUTDIR    := wiiu_debug_build
endif

BUILD       :=  $(OUTPUTDIR)
TARGET		:=	$(OUTPUTDIR)/wiilinku
SOURCES		:=	src src/wiiu
DATA		:=	data
INCLUDES	:=	src src/wiiu

#-------------------------------------------------------------------------------
# options for code generation
#-------------------------------------------------------------------------------
CFLAGS   := -D__WIIU__ -D__WUT__ -DWLU_CLIENT \
	-DWLU_VERSION_STR=\"$(WLU_VERSION_STR)\" \
	$(INCLUDE) $(MACHDEP) -Wall -Wextra 

ifeq ($(BUILD_TYPE),Release)
CFLAGS	 +=	-O2 -DNDEBUG -DWLU_DEBUG
else
CFLAGS   += -O0 -g -DDEBUG -DWLU_DEBUG
endif

CXXFLAGS := $(CFLAGS)

ifeq ($(BUILD_TYPE),Debug)
ASFLAGS := -g
LDFLAGS  = -g
endif

ASFLAGS	+=	$(ARCH)
LDFLAGS	+=  $(ARCH) $(RPXSPECS) -Wl,-Map,$(notdir $*.map)

LIBS     := -lwut

#-------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level
# containing include and lib
#-------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS) $(WUT_ROOT)


#-------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#-------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#-------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export TOPDIR	:=	$(CURDIR)

export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
			$(foreach dir,$(DATA),$(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.*)))

#-------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#-------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
#-------------------------------------------------------------------------------
	export LD	:=	$(CC)
#-------------------------------------------------------------------------------
else
#-------------------------------------------------------------------------------
	export LD	:=	$(CXX)
#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------

export OFILES_BIN	:=	$(addsuffix .o,$(BINFILES))
export OFILES_SRC	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES 	:=	$(OFILES_BIN) $(OFILES_SRC)
export HFILES_BIN	:=	$(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
			$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
			-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#-------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/wiiu.mak

#-------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD)/*.o $(BUILD)/*.d

#-------------------------------------------------------------------------------
else
.PHONY:	all

DEPENDS	:=	$(OFILES:.o=.d)

#-------------------------------------------------------------------------------
# main targets
#-------------------------------------------------------------------------------
all	:	$(OUTPUT).rpx

$(OUTPUT).rpx	:	$(OUTPUT).elf
$(OUTPUT).elf	:	$(OFILES)

$(OFILES_SRC)	: $(HFILES_BIN)

#-------------------------------------------------------------------------------
# you need a rule like this for each extension you use as binary data
#-------------------------------------------------------------------------------
%.bin.o	%_bin.h :	%.bin
#-------------------------------------------------------------------------------
	@echo $(notdir $<)
	@$(bin2o)

-include $(DEPENDS)

#-------------------------------------------------------------------------------
endif
#-------------------------------------------------------------------------------
