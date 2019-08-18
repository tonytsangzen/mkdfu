# mkdfu

(GPL) Generate Dfu file for stm32
 
This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.

## Instructions


download the software :

	git clone git@github.com:tonytsangzen/mkdfu.git
	cd mkdfu
	make
	./mkdfu -l layout/stm32f411x.json -o out.dfu

## Layout file format
	key [name]	: indicated session(target or element) name, can avoid.
	key [source]	: idicated element source file. just support binary file currently.
	key [address]	: idicated element download address.	
	key [size]	: max download size 
	key [essential] : if element must present, set this key to true.
 	[for mor infomation, see DfuSe File Format Specification](http://rc.fdr.hu/UM0391.pdf)
## License

[GPLv3](https://github.com/Keidan/hex2bin/blob/master/LICENSE)
