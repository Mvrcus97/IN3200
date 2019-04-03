README FOR PARTIAL EXAM - IN3200 High Performance Computing

(https://www.uio.no/studier/emner/matnat/ifi/IN3200/v19/teaching-material/in3200_in4200_partial_exam_v19.pdf)


The algorithm is divided into 3 parts.

1) Reading the text file
2) Giving each webpage a score
3) Finding the top N web pages.



1) Reading the text file

	I)  Go through the text file, count all edges.
	II) Go through the file once more. Use the
	    information found to create the CRS in addition
	    to a "boolean" array of which nodes are dangling
	    web pages.The CRS exists as three 1-Dimensional arrays. 

	III) Return the CRS and the Dangling array. 


2) Giving each webpage a score. 

	I've included a bunch of sequential functions for a deeper
	understanding of the different algorithms. These were the
	initial implementation before i parallelized using OpenMP.
	
	Go through a number of iterations of the calculation of X^k
	untill we've reached a threshold. For each iteration:
	
	I)  Begin by finding the "W", which is equal to the left
	    side of the equation for X^k. Add the result to x_new[0..Nodes]
	II) Compute the sparse matrix multiplication, Ax^(k-1). Sum the
	    result together with part I) to complete the calculation. 

	III) Pointer-swapping from x[] and x_new[]. Also check if we have
	     converged yet. Iteration done.


3) Finding the top N web pages.

	I realized it is not neccesary to sort all webpages if we want
	to find the top N pages. A more efficient way of doing this can
	be done by the so called top-K-sort. For more info read the report.

	I)  Sort the first K web pages. When sorting we also have to keep
	    track of the web pages' initial number, or ID if you'd like. 
	II) For each next web page, check if has a higher score than the
	    lowest top-k- pages. If yes, swap positions and re-sort the
	    top-k- results. 
	III) Print the top N pages found. Also return an array of the top scores.



HOW TO RUN. 
Run the program by ./run 'file_name' 'damping_factor' 'epsilon' 'top_n_webpages'. 
If you want to run the NotreDame file with a damping factor of 0.85, epsilon of 
0.00001 and also find the top 10 web pages, you would run:

./run web-NotreDame.txt 0.85 0.00001 10





		
