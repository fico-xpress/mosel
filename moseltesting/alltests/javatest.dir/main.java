/*******************************************************
   Mosel Testing Examples 
   ====================== 

   Running the Java program myjavatest. 

   This file demoes the use of 'moseltest' tags within Java programs:
   * 'build' serves to build the Java program
   * 'outexpect' checks output produced by the Java program and any Mosel 
     models that are run by it

!*! build myjavatest.class
!*! outexpect Numbers generated\: 14
!*! outexpect Found 14 numbers

   author: S.Heipcke, Nov 2022

  (c) Copyright 2022 Fair Isaac Corporation

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

********************************************************/
public class main
{
 public static void main(String[] args) throws Exception
 {
   myjavatest.main(args);
 }  
}
