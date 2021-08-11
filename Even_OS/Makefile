TOOLPATH = ../z_tools/
INCPATH  = ../z_tools/haribote/

MAKE     = $(TOOLPATH)make.exe -r
EDIMG    = $(TOOLPATH)edimg.exe
IMGTOL   = $(TOOLPATH)imgtol.com
COPY     = copy
DEL      = del

# �f�t�H���g����

default :
	$(MAKE) haribote.img

# �t�@�C�������K��

haribote.img : haribote/ipl09.bin haribote/haribote.sys Makefile \
		noodle/noodle.hrb \
		type/type.hrb game/game.hrb\
		bball/bball.hrb invader/invader.hrb \
		calc/calc.hrb tview/tview.hrb gview/gview.hrb 

	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:haribote/ipl09.bin len:512 from:0 to:0 \
		copy from:haribote/haribote.sys to:@: \
		copy from:ipl09.nas to:@: \
		copy from:make.bat to:@: \
		copy from:noodle/noodle.hrb to:@: \
		copy from:game/game.hrb to:@: \
		copy from:type/type.hrb to:@: \
		copy from:bball/bball.hrb to:@: \
		copy from:invader/invader.hrb to:@: \
		copy from:calc/calc.hrb to:@: \
		copy from:tview/tview.hrb to:@: \
		copy from:gview/gview.hrb to:@: \
		copy from:pictdata/fujisan.jpg to:@: \
		copy from:pictdata/night.bmp to:@: \
		copy from:nihongo/nihongo.fnt to:@: \
		copy from:heloos.jpg to:@: \
		copy from:picture/3.jpg to:@: \
		copy from:picture/2.jpg to:@: \
		copy from:picture/5.jpg to:@: \
		imgout:haribote.img

# �R�}���h

run :
	$(MAKE) haribote.img
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) haribote.img
	$(IMGTOL) w a: haribote.img

full :
	$(MAKE) -C haribote
	$(MAKE) -C apilib
	$(MAKE) -C noodle
	$(MAKE) -C type
	$(MAKE) -C bball
	$(MAKE) -C invader
	$(MAKE) -C calc
	$(MAKE) -C tview
	$(MAKE) -C gview
	$(MAKE) -C game
	$(MAKE) haribote.img

run_full :
	$(MAKE) full
	$(COPY) haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install_full :
	$(MAKE) full
	$(IMGTOL) w a: haribote.img

run_os :
	$(MAKE) -C haribote
	$(MAKE) run

clean :
# �������Ȃ�

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img

clean_full :
	$(MAKE) -C haribote		clean
	$(MAKE) -C apilib		clean
	$(MAKE) -C noodle		clean
	$(MAKE) -C type			clean
	$(MAKE) -C bball		clean
	$(MAKE) -C invader		clean
	$(MAKE) -C calc			clean
	$(MAKE) -C tview		clean
	$(MAKE) -C game		clean
	$(MAKE) -C gview		clean

src_only_full :
	$(MAKE) -C haribote		src_only
	$(MAKE) -C apilib		src_only
	$(MAKE) -C noodle		src_only
	$(MAKE) -C type			src_only
	$(MAKE) -C bball		src_only
	$(MAKE) -C invader		src_only
	$(MAKE) -C calc			src_only
	$(MAKE) -C tview		src_only
	$(MAKE) -C game		src_only
	$(MAKE) -C gview		src_only
	-$(DEL) haribote.img

refresh :
	$(MAKE) full
	$(MAKE) clean_full
	-$(DEL) haribote.img
