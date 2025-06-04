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
   ![alt text](image-14.png)
## Planned Features

1. cat on tags , commits , trees
2. commit
3. push stuff to github
4. refactor code
5. add error handling
    - read whats the proper way to do error handling not just random try catch blocks
6. singleton class
    - other patterns learn & see how to refactor accordingly


### Prerequisites
- C++17
- zlib for compression
- openssl for sha1