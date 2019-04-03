README FOR PARTIAL EXAM - IN3200 High Performance Computing

(https://www.uio.no/studier/emner/matnat/ifi/IN3200/v19/teaching-material/in3200_in4200_partial_exam_v19.pdf)


The algorithm is divided into 3 parts.

1) Reading the text file
2) Giving each webpage a score
3) Finding the top N web pages.

For more information read the report. 

How to compile

I've included a makefile. Compile the program by typing 'make'. 
The makefile uses the gcc- compiler, with some additional flags such as -Wall and -fopenmp


How to run

Run the program by ./run 'file_name' 'damping_factor' 'epsilon' 'top_n_webpages' ‘threads’

If you want to run the NotreDame file with a damping factor of 0.85, epsilon of 
0.00001 ,find the top 10 web pages and use all avaible threads on your current computer, you would run:

./run web-NotreDame.txt 0.85 0.00001 10 0





		
