/*******************************************************
   Mosel Testing Examples 
   ====================== 

   file myjavatest.java
   ````````````````````
   Exchanging data between a Mosel model and Java host application.
   - Callbacks for exchanging data (sparse data, integer indices) -
   
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

import com.dashoptimization.*;

public class myjavatest
{
 // Class to receive solution values
 public static class MySol
 {
  public int ind;                   // index value
  public double val;                // result value
 }
 static MySol[] solution;
 static int solsize;
 
 // A class to exchange data with model via callbacks
 public static class modelInit implements XPRMInitializationTo
 {
  /**** Retrieving data from Mosel ****/
  public boolean initializeTo(String label, XPRMValue value)
  {
   XPRMArray solarr;
   XPRMSet[] sets;
   int[] indices;
   int ct;

   if(label.equals("Res"))
   {
    solarr=(XPRMArray)value;
    solsize=solarr.getSize();
    solution = new MySol[solsize];
    for(int i=0;i<solsize;i++) solution[i] = new MySol();

    sets = solarr.getIndexSets();          // Get the indexing sets
    ct=0;
    indices = solarr.getFirstTEIndex();    // Get the first entry of the array
    do
    {
     solution[ct].ind=sets[0].getAsInteger(indices[0]);
     solution[ct].val=solarr.getAsReal(indices);
     ct++;
    } while(solarr.nextTEIndex(indices));  // Get the next index  
   }
   else System.out.println("Unknown output data item: " + label + "=" + value);
   return true;
  }
 }

 // Interface objects are static: no need to bind
 public static modelInit cbinit=new modelInit();

 public static void main(String[] args) throws Exception
 {
  XPRM mosel;
  XPRMModel mod;  

  mosel = new XPRM();                 // Initialize Mosel

  mosel.compile("calctest.mos");      // Compile & load the model
  mod = mosel.loadModel("calctest.bim");

  // File names are passed through execution parameters
  mod.execParams = "NUM=200,SOLFILE='java:myjavatest.cbinit'";

  mod.run();                          // Run the model

  // Display solution values obtained from the model
  System.out.println("Found " + solsize + " numbers");
  for(int i=0;i<solsize;i++)
   System.out.println(" " + solution[i].ind + "^2 = " + solution[i].val);

  mod.reset();                        // Reset the model
 }
}

