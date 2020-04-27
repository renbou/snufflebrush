### Usage:  
  ./snufflebrush -e -i <input> -o <output> -f <file> -p <password>  
  ./snufflebrush -d -i <input> -o <output> -p <password>  
  
Specify -e for **encoding**, -d for **decoding**.  
#### Encoding:  
-i input file (png image) in which to hide the data  
-o output image file with hidden data  
-f the file to hide  
-p password which will be used to generate a sequence for hiding the file  

#### Decoding:  
-i input file (png image) with the hidden data  
-o output file (extracted data with password)  
-p password which will be used to extract the file  
