# 사용 오픈 라이브러리
https://github.com/PPODO/Server_Library  
https://www.boost.org/  
https://flatbuffers.dev/  
https://slproweb.com/products/Win32OpenSSL.html  
https://github.com/yourWaifu/sleepy-discord  
https://github.com/open-source-parsers/jsoncpp

# 디스코드 커맨드
/ping name:(프로세스 이름)

# 테스트 서버 접속 방법
주소와 포트 번호는 아래와 같습니다.  
![image](https://github.com/user-attachments/assets/3c57e35c-1957-412e-ba0d-62a632cb24ff)


**그리고 시작 맵은 무족권 LoginMap이여야함.**  
![image](https://github.com/user-attachments/assets/bb0b9101-fb8f-4fee-8832-108d65f62999)


## 1
**게임 인스턴스는 BP_MFGameInstance로 설정해줍니다.**  
![image](https://github.com/user-attachments/assets/380522a2-9f06-4708-9731-0665ca2f4657)


## 2
**콘텐츠 폴더에 BP_MFGameInstance가 존재합니다.**   
![image](https://github.com/user-attachments/assets/a9f85ca6-3787-4ff2-9ffa-48c15dd17eb1)


## 3
**열어보시면 아래와 같이 게임 맵 이름을 넣을 수 있습니다.**  
![image](https://github.com/user-attachments/assets/93bdea62-704f-4aa6-b67d-6466d7f2f554)


## 4
**콘텐츠 폴더에서 맵 에셋 우클릭하시면, 레퍼런스 복사 있을건데 클릭하셔서 Game Name Name 프로퍼티에 적어주시면 됩니다.**  
**네모칸 내용만 빼서 적어주시면 됨.**  
![image](https://github.com/user-attachments/assets/b7b588cd-1611-48d3-a048-16e5ae4d9a53)


## 5
**테스트에 사용될 게임모드는 MFGameModeBase를 상속받게 만들어주세요.**  
![image](https://github.com/user-attachments/assets/8b9def29-d4b8-4857-b5d4-293435f1a31c)


## 6
**이것만 하시면 따로 뭐 설정할 필요없이 접속 가능.**
