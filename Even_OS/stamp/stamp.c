#include "apilib.h"
#include <stdio.h>

void HariMain(void)
{
    api_putstr0("input 4 stamp values:\n");

    char cmd[32],result[20],*cmd_str;
    int  i, j, k, l,p,q;
    int a[4];
    static int s[1000];  /*邮资*/
    int x,y,r=0,count=0; //r用来做移动指针工作,x指向数字开头，y指向连续数字结尾
    api_cmdline(cmd,40);

    api_putstr0(cmd);
//     for (cmd_str = cmd; *cmd_str > ' '; cmd_str++) { }	
//     for (; *cmd_str != 0; )
// {

// if ('0' <= cmd_str[r] && cmd_str[r] <= '9')//是数字
// {
//     p=r;
//     q=r+1;
//     a[count]=cmd_str[r]-'0';
//     while('0' <= cmd_str[q] && cmd_str[q] <= '9')
//     {
//         a[count]=10*a[count]+(cmd_str[q]-'0');
//         q++;
//     }
//     r=q;  //新起点
//     count++; //count应该为4

// }
// else r++;

// }
//     //scanf("%d %d %d %d", &a, &b, &c, &d);  /*输入四种面值邮票*/
//     for(i=0; i<=5; i++)  /*循环变量i用于控制a分面值邮票的张数，最多5张*/
//         for(j=0; i+j<=5; j++)  /*循环变量j用于控制b分面值邮票的张数，a分邮票+b分邮票最多5张*/
//             for(k=0; k+i+j<=5; k++)  /*循环变量k用于控制c分面值邮票的张数，a分邮票+b分邮票+c分邮票最多5张*/
//                 for(l=0; k+i+j+l<=5; l++)  /*循环变量l用于控制d分面值邮票的张数,a分邮票+b分邮票+c分邮票+d分邮票最多5张*/
//                     if( a[0]*i+a[1]*j+a[2]*k+a[3]*l )
//                         s[a[0]*i+a[1]*j+a[2]*k+a[3]*l]++;
//     for(i=1; i<=1000; i++)
//         if( !s[i] )
//             break;

//     sprintf(result, "The max is %d\n", --i);
//     api_putstr0(result);
    api_end();
    //printf("The max is %d.\n", --i);
}

