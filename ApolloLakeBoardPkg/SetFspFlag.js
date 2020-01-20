/* @file

******************************************************************************
* Copyright (c) 2016, Insyde Software Corp. All Rights Reserved.
*
* You may not reproduce, distribute, publish, display, perform, modify, adapt,
* transmit, broadcast, present, recite, release, license or otherwise exploit
* any part of this publication in any form, by any means, without the prior
* written permission of Insyde Software Corporation.
*
******************************************************************************
*/

//
// Define variables
//
var String = /^DEFINE FSP_ENABLE/;

//
// Loop all string array and output file
//
var ForReading = 1, ForWriting = 2, ForAppending = 8;
var fso = new ActiveXObject("Scripting.FileSystemObject");
//
// Read whole file into string array
//
var InputFileFp = fso.OpenTextFile("Project.env", ForReading, false);
var InputStream = InputFileFp.ReadAll().split("\r\n");
InputFileFp.Close();

File = fso.OpenTextFile("Project.env", ForWriting, true);
var Count;
var RegExpResult;
var InDxeModules = 0;

for (Count = 0; Count < InputStream.length; Count ++) {
  if ((RegExpResult = String.exec(InputStream[Count])) != null) {
    File.WriteLine("DEFINE FSP_ENABLE = YES");
  } else {
    File.WriteLine(InputStream[Count]);    
  }
}
File.Close();
