@goto code
������ע��

setlocal enabledelayedexpansion
�ο�http://blog.csdn.net/kolamemo/article/details/18036021

������Ŀ¼
for /d %%i in (*) do (set val=%%i && ren "!val!" "!val:~1,-1!")

�ַ�����ȡ
set ifo=abcdefghijklmnopqrstuvwxyz0123456789
echo ��ȡǰ5���ַ���
echo %ifo:~0,5%
echo ��ȡ���5���ַ���
echo %ifo:~-5%
echo ��ȡ��һ����������6���ַ���
echo %ifo:~0,-5%
:code

@setlocal enabledelayedexpansion
@rem ����Ŀ¼
@for /d %%i in (*) do (set val=%%i && echo !val!)
@pause