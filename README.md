# Custom Git Implementation

A WIP implementation of a simplified Git client in C++. 

## directory looks like this before executing any commands 
![before git init](image.png)

## Implemented Features

1. **git init**  
   ![Git init output](img/image-1.png)

2. **git hash-object file**
   ![Hash-object output](img//image-2.png)

3. **git find-object hash**  
  
   ![Find-object output](img//image-3.png)

4. **git cat-object hash**  
   Displays the contents of a Git object (currently supports blobs only).  
   ![Cat-object output](img/image-4.png)

5. **git add**  
   ![Git add output](img/image-5.png)  
   ![Git add output](img/image-6.png)  
   ![Git add output](img/image-7.png)  
   ![Git add output](img/image-8.png)

6. **git ls-files**  
   ![Ls-files output](img/image-9.png)

7. **git status**  
   Shows the status of the working directory and staging area.  
   *Before changes*:  
   ![Status before changes](img/image-10.png)  
   *After changes* (removed `features.txt`, added `f3.txt`, modified `f1.txt`):  
   ![Status after changes](img/image-11.png)

7. **git diff**
   *changes* (edited `f1.txt`):    
![alt text](img/image-13.png)

8. **git write-tree**
   Writes tree 
   ![alt text](img/image-14.png)

9. **git commit**
   ![alt text](img/image-15.png)

## Planned in Future

1. refactor code
2. add error handling - ignored all / any edge cases for now 
3. singleton class
    - learn other design patterns & refactor accordingly

## learnings
1. Would have been better to add test / edge cases handling from the start , now I have forgotten what assumptions i took . Never doing this again 
2. Learnt c++ while implementing this , fun :) , now i think i will pick up a book 
3. Deeper understanding of how GIT works under the hood. I simplified some optimisations for storing index . Did not add multi branch feature for now.

### Prerequisites
- C++17
- zlib for compression
- openssl for sha1