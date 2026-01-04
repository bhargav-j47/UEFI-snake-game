CC      = clang
LD      = lld-link
TARGET  = game.efi

SRCS    = game.c
OBJS    = $(SRCS:.c=.o)

EFIINC  = /usr/include/efi

CFLAGS  = -target x86_64-unknown-windows \
          -I$(EFIINC) -I$(EFIINC)/x86_64 -I$(EFIINC)/protocol \
          -fno-stack-protector -fshort-wchar -mno-red-zone \
          -Wall -c

LDFLAGS = -subsystem:efi_application -entry:efi_main


all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -out:$(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.efi
