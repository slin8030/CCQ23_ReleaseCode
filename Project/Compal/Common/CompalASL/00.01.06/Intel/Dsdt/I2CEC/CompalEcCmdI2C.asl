/*
/*
 * (C) Copyright 2012-2020 Compal Electronics, Inc.
 *
 * This software is the property of Compal Electronics, Inc.
 * You have to accept the terms in the license file before use.
 *
 * Copyright 2011-2012 Compal Electronics, Inc.. All rights reserved.

 By installing or using this software or any portion thereof, you ("You") agrees to be bound by the following terms of use ("Terms of Use").
 This software, and any portion thereof, is referred to herein as the "Software."

 USE OF SOFTWARE.  This software is the property of Compal Electronics, Inc. (Compal) and is made available by Compal to You, and may be used only by You for personal or project evaluation.

 RESTRICTIONS.  You shall not claim the ownership of the Software and shall not sell the Software. The software shall be distributed as pre-installed software incorporated in the devices manufactured by Compal only, and shall not be distributed separately via internet or any other medium.

 INDEMNITY.  You agree to hold harmless and indemnify Compal and Compal¡¦s subsidiaries, affiliates, officers, agents, and employees from and against any claim, suit, or action arising from or in any way related to Your use of the Software or Your violation of these Terms of Use, including any liability or expense arising from all claims, losses, damages, suits, judgments, litigation costs and attorneys' fees, of every kind and nature.
 In such a case, Compal will provide You with written notices of such claim, suit, or action.

 DISCLAIMER.  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 R E V I S I O N    H I S T O R Y
 
 Ver       Date      Who          Change
 --------  --------  --------     ----------------------------------------------------
 1.00                             Init version.
*/

Scope(COMMON_ASL_EC_PATH){

    Mutex(CFMX,0x0)
    Name(ECRD, Buffer(0x0A){0,6,0,0,0,0,0,0,0,0}) 
    CreateByteField(ECRD, 0x00, ERST) // STAT = Status (Byte)
    CreateByteField(ECRD, 0x01, ERLN) // length
    CreateByteField(ECRD, 0x02, ECR0) // Return Data0
    CreateByteField(ECRD, 0x03, ECR1) // Return Data1 
    CreateByteField(ECRD, 0x04, ECR2) // Return Data2 
    CreateByteField(ECRD, 0x05, ECR3) // Return Data3
    CreateByteField(ECRD, 0x06, ECR4) // Return Data4 
    CreateByteField(ECRD, 0x07, ECR5) // Return Data5 

    Name(ECWR, Buffer(0x0A){0,5,0,0,0,0,0,0,0,0}) 
    CreateByteField(ECWR, 0x00, ECST) // STAT = Status (Byte)
    CreateByteField(ECWR, 0x01, ECLN) // length
    CreateByteField(ECWR, 0x02, ECD0) // EC Data0  (EC command)
    CreateByteField(ECWR, 0x03, ECD1) // EC Data1 
    CreateByteField(ECWR, 0x04, ECD2) // EC Data2
    CreateByteField(ECWR, 0x05, ECD3) // EC Data3
    CreateByteField(ECWR, 0x06, ECD4) // EC Data3
  
    Method(EC00)// Clean ECRD ECWR buffer
    {
      Store(0x0,  ECD0)
      Store(0x0,  ECD1)
      Store(0x0,  ECD2)
      Store(0x0,  ECD3)
      Store(0x0,  ECD4)
      
      Store(0x0,  ECR0)
      Store(0x0,  ECR1)
      Store(0x0,  ECR2)
      Store(0x0,  ECR3)
      Store(0x0,  ECR4)
      Store(0x0,  ECR5)          
    }

    /********************************************** 
    Function Method for EC namecespaec read 
    
    Input:
        arg0 : EC namespace offset(SPEC:address)
    Output:
        ECRD Buffer 
    Notes:
       I2C Read Protocol:
       |WR slv addr|Data 0       |RD slv addr |Data 0      | Data 1     |Data 2      |Data 3      |Data 4      |Data 5      |    
       |0xE0       |0x01(Fixed)  |0xE1	      |Return Data0|Return Data1|Return Data2|Return Data3|Return Data4|Return Data5|			

    **********************************************/
    Method(ECNR,1)
    { 
      Acquire(CFMX, 0xFFFF)
      EC00()                                                    // Clean ECRD ECWR buffer
      Store(0x80, ECD0)                                         // Set the read ec namespace cmd (0x80) to data0
      Store(arg0, ECD1)                                         // write namespace offset to ECD1
      Store(Store(ECWR,COMMON_ASL_EC_PATH.I2CW),ECWR)           // Send EC command/Data 
      if(LEqual(ECST, 0x00)){                                   // Check Success?
        Store(COMMON_ASL_EC_PATH.I2CR, ECRD)                    // Read EC return data 
      }
      Release (CFMX)
    }

    /********************************************** 
    Function Method for EC namecespaec write
    
    Input:
        arg0 : EC namespace offset(SPEC:address)
    Output:
        none
    Notes:
       I2C Write Protocol
       |Slave Write Address   |DATA0      |DATA       |DATA2  |DATA3  |DATA4  |DATA5
       |0xE0                  |0x02       |EC command	|Data0	|Data1	|Data2  |Data3

    **********************************************/
    Method(ECNW,2)
    { 
      Acquire(CFMX, 0xFFFF)
      EC00()                                                    // Clean ECRD ECWR buffer
      Store(0x81, ECD0)                                         // Set the write ec namespace cmd (0x80) to data0
      Store(arg0, ECD1)                                         // write namespace offset to ECD0
      Store(arg1, ECD2)                                         // write data to ECD1
      Store(Store(ECWR,COMMON_ASL_EC_PATH.I2CW),ECWR)           // Send EC command/Data 
      Release (CFMX)
    }
    
    /********************************************** 
    Function Method for Write any ec command
    
    Input:
        arg0 : EC command
        arg1 : EC data0
        arg2 : EC data1
        arg3 : EC data2
        arg4 : EC data3

    Output:
        ECRD 

    Example:
        ECRW(0x41, 0xA1, 0x00, 0x00, 0x00)        // get platform id
    **********************************************/
    Method(ECRW,5)
    {
      Acquire(CFMX, 0xFFFF)

      // clear all of status
      COMMON_ASL_EC_PATH.EC00()

      // Store command and data 
      Store(arg0, ECD0)
      Store(arg1, ECD1)
      Store(arg2, ECD2)
      Store(arg3, ECD3)
      Store(arg4, ECD4)

      Store(Store(COMMON_ASL_EC_PATH.ECWR, COMMON_ASL_EC_PATH.I2CW),COMMON_ASL_EC_PATH.ECWR)
      If(LEqual(COMMON_ASL_EC_PATH.ECST, 0x00)){    
        Store(COMMON_ASL_EC_PATH.I2CR, COMMON_ASL_EC_PATH.ECRD) 
      }
      Release (CFMX)
    }
}
