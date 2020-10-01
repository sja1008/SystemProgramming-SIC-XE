```
  ____  ____                    _     _  _  _____                 
 / ___||  _ \   _ __  _ __ ___ (_)  _| || ||___ /                  
 \___ \| |_) | | '_ \| '__/ _ \| | |_  ..  _||_ \                  
  ___) |  __/  | |_) | | | (_) | | |_      _|__) | May. 21st, 2020 
 |____/|_|     | .__/|_|  \___// |   |_||_||____/  Jina Song      
               |_|           |__/                 
```               
   
   
> Shell 명령어는 소문자로 입력하세요.
> 인자로 사용되는 숫자는 16 진수로 입력하세요.
> Assembler가 처리하는 파일의 알파벳은 모두 대문자입니다.
> 16 진수 입력 시에는 알파벳 대소문자 모두 사용 가능합니다.
> 메모리 주소는 0x0부터 0xFFFFF까지 범위까지 유효합니다.
> progaddr은 0으로 초기화되어 있습니다.
   
   
   
**Compile**   
    `make`를 입력하여 컴파일합니다.   
   
**Execute**   
    `./20160563.out`을 입력하여   
    컴파일 시 생성된 *20160563.out* 파일을 실행합니다.   
   
**Clean**   
    `make clean`을 입력하여   
    컴파일 시 생성된 이진 파일들을 삭제합니다.   
   
**Commands**   
    입력 프롬프트*sicsim>*에 다음과 같은 명령어들을 입력할 수 있습니다.   
    - h[elp] : 명령어들의 목록을 출력   
    - d[ir] : 현재 디렉토리의 파일을 출력   
    - q[uit] : sicsim 을 종료   
    - hi[story] : 현재까지 입력한 유효한 명령어들을 출력   
    - du[mp][start,end] : 메모리의 내용을 출력   
    - e[dit]address,value : 메모리 *address* 번지의 값을 *value*로 변경   
    - f[ill]start,end,value : 메모리 *start*부터 *end*번지까지의 값을 *value*로 변경   
    - reset : 메모리 전체의 값을 0으로 변경   
    - opcode mnemonic : *mnemonic*의 opcode를 출력   
    - opcodelist : opcode hash table의 내용을 출력   
    - assemble filename : *filename*을 assemble 하여 .obj 파일과 .lst 파일을 생성   
    - type filename : 현재 디렉토리의 *filename*을 읽어 출력   
    - symbol : 가장 최근에 성공한 assemble 과정에서 생성된 symbol table을 출력   
    - progaddr address : *loader* 또는 *run* 명령어 수행을 시작할 주소를 설정   
    - loader filename1 [filename2 filename3] : *filename*에 해당하는 최대 3개의 .obj 파일들을 읽어 linking loader를 수행   
    - bp address : 해당 *address*에 break point를 지정   
    - bp clear : 설정한 모든 break points를 삭제   
    - bp : 설정된 모든 break points를 출력   
    - run : *loader* 명령어로 메모리에 load된 프로그램을 실행   
    
