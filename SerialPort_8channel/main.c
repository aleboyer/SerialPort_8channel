//
//  main.c
//  SerialPort_8channel
//
//  Created by Arnaud Le Boyer on 1/17/17.
//  Copyright © 2017 Arnaud Le Boyer. All rights reserved.
//


#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

struct termios  config;
uint32_t numChannel = 8;

int main() {
    int fReadData; FILE *fileData;FILE *fileDatadouble;
    //    const char *device = "/dev/tty.usbserial-FTYVXOH6";
    const char *device = "/dev/tty.usbserial-FTXOY2MY";
    
    uint8_t buffer=0;
    uint32_t biggerBuffer;
    uint8_t * buf_ptr;
    int32_t count_endian;
    
    fReadData  = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
    //   fReadData  = open(device, O_RDWR | O_NOCTTY );
    
    fileData=fopen("/Users/aleboyer/ARNAUD/SCRIPPS/SHEAR_PROBE/julia/READ_EPSILOMETER/data_c_streaming.txt","w+");
    fileDatadouble=fopen("/Users/aleboyer/ARNAUD/SCRIPPS/SHEAR_PROBE/julia/READ_EPSILOMETER/data_c_streaming_double.txt","w+");
    
    if(fReadData == -1) {
        printf( "failed to open port\n" );
    }
    else{
        printf( "fd = %i\n",fReadData );
    }
    
    if(!isatty(fReadData)) {printf(" fd is not a tty ");}
    else{
        printf(" fd is a tty \n");
    };
    
    
    if(tcgetattr(fReadData, &config) < 0) { printf(" no tty configuration");}
    else{
        printf("config.c_iflag=%lx\n",config.c_iflag);
        printf("config.c_oflag=%lx\n",config.c_oflag);
        printf("config.c_cflag=%lx\n",config.c_cflag);
        printf("config.c_lflag=%lx\n",config.c_lflag);
        printf("config.c_ispeed=%lx\n",config.c_ispeed);
        printf("config.c_ospeed=%lx\n",config.c_ospeed);
    }
    
    config.c_lflag &= ICANON ;
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;
    
    printf("config.c_lflag=%lx\n",config.c_lflag);
    printf("config.c_cflag=%lx\n",config.c_cflag);
    
    
    if(config.c_ispeed != B115200 || config.c_ispeed != B115200) {
        printf(" baud rate is not 446800\n");
        //config.c_ispeed = B115200;
        //config.c_ospeed = B115200;
        config.c_ispeed = 460800;
        config.c_ospeed = 460800;
        printf("config.c_ispeed=%lx\n",config.c_ispeed);
        printf("config.c_ospeed=%lx\n",config.c_ospeed);
        printf(" now baud rate is 460800\n");
        
        
    }
    else{
        printf(" baud rate is 115200\n");
    }
    
    //the configuration is changed any data received and not read will be discarded.
    tcsetattr(fReadData, TCSAFLUSH, &config);
    
    
    int state = 0 ;
    uint32_t i;
    uint32_t test=1;
    while(test){
            switch (state) {
                case 0 :  // First state : wait for 0x1e read 32 bytes and check 0x1e
 //                    printf("coucou, state = %d\n",state);
                     if (read(fReadData,&buffer,1)>0){
//                        printf("coucou, state = %d\n",state);
                         //printf("coucou start case 0, state = %d\n",state);

                        if (buffer==0x1e){
                            printf("%02x\n",buffer);
//                            printf("coucou, state = %d\n",state);
                            for (i=1;i<=numChannel;i++){
                                buf_ptr      = (uint8_t*) &biggerBuffer;
                                biggerBuffer = 0;
                                count_endian = 2;
                                // the port is open as non block
                                // we nee a while to hang in the loop until the new bye can be read
                                while(count_endian>=0){
                                    if (read(fReadData,&buffer,1)>0){
                                        //printf("%d bytes = %01x\n",count_endian,buffer);
                                        buf_ptr[count_endian] = buffer;
                                        count_endian--;
                                    } // end if read buffer
                                } //end while count endian
                                //printf("%08x\n",biggerBuffer);
    //                            printf("numchannel %d\n",i);
                                
                            }
                            
                            // read time stamp
                            count_endian = 3;
                            buf_ptr      = (uint8_t*) &biggerBuffer;
                            biggerBuffer = 0;
                            while (count_endian>=0){  // need a while loop until the bytes arrive
                                if (read(fReadData,&buffer,1)>0){
                                    //printf("%d bytes = %01x\n",count_endian,buffer);
                                    buf_ptr[count_endian] = buffer;
                                    count_endian--;
                                } // end if read buffer
                            } //end while count endian
                            //printf("%08x\n",biggerBuffer);
 
                            
                            // get the next byte and check if this 0x1e so that we are sure to be sync to get the sample
//                            printf("coucou, state = %d",state);
                            count_endian = 1;
                            while (count_endian>=0){  // need a while loop until the bytes arrive
                                if (read(fReadData,&buffer,1)>0){
                                    printf("%x\n",buffer);
                                    if (buffer==0x1e){
                                        state=1;i=0;
                                        //printf("coucou end case 0, state = %d\n",state);
                                        //test=0;
                                        count_endian--;
                                        break;
                                    }
                                    else{
                                        //test=0;
                                        //printf("souci should be 0x1e");
                                        count_endian--;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
        
                case 1 :
                    //printf("enter case 1\n");
                    //printf("%02x\n",buffer);
                    for (i=1;i<=numChannel;i++){
                        buf_ptr      = (uint8_t*) &biggerBuffer;
                        biggerBuffer = 0;
                        count_endian = 2;
                        // the port is open as non block
                        // we nee a while to hang in the loop until the new bye can be read
                        while(count_endian>=0){
                            if (read(fReadData,&buffer,1)>0){
                                buf_ptr[count_endian] = buffer;
                                count_endian--;
                            } // end if read buffer
                        } //end while count endian
                        //printf("%08x\n",biggerBuffer);
                        fprintf(fileData,"0x%08x,",biggerBuffer);  // file print
                        fprintf(fileDatadouble,"%9u,",biggerBuffer);  // file print
                        fflush(fileData);
                        fflush(fileDatadouble);
                    }
                    // read time stamp
                    count_endian = 3;
                    buf_ptr      = (uint8_t*) &biggerBuffer;
                    biggerBuffer = 0;
                    while (count_endian>=0){  // need a while loop until the bytes arrive
                        if (read(fReadData,&buffer,1)>0){
                            //printf("%d bytes = %01x\n",count_endian,buffer);
                            buf_ptr[3-count_endian] = buffer;
                            count_endian--;
                        } // end if read buffer
                    } //end while count endian
                    //printf("%08x\n",biggerBuffer);
                    fprintf(fileData,"0x%08x\n",biggerBuffer);  // file print
                    fprintf(fileDatadouble,"%9d\n",biggerBuffer);  // file print
                    fflush(fileData);
                    fflush(fileDatadouble);

                    
                    count_endian = 1;
                    while (count_endian>=0){  // need a while loop until the bytes arrive
                        if (read(fReadData,&buffer,1)>0){
                            //printf("%02x\n",buffer);
                            if (buffer==0x1e){
                                i=0;
                                //printf("coucou end case 1, state = %d\n",state);
                                count_endian--;
                                //test=0;
                                break;
                            }
                            else{
                                printf("state 1 mais ca deconne");
                                state=0;
                                break;
                            }
                        }
                    }
                    break;
            }
        
    } // while(1)
} //end main

