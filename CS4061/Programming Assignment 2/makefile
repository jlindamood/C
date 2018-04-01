make: Vote_Counter Leaf_Counter Aggregate_Votes
	gcc Vote_Counter.c -o Vote_Counter -w

Leaf_Counter: Leaf_Counter.c
	gcc Leaf_Counter.c -o Leaf_Counter -w
	

Aggregate_Votes: Aggregate_Votes.c Leaf_Counter
	gcc Aggregate_Votes.c -o Aggregate_Votes -w
	

Vote_Counter: Vote_Counter.c Aggregate_Votes Leaf_Counter
	gcc Vote_Counter.c -o Vote_Counter -w
	

clean:
	-rm -f Vote_Counter
	-rm -f Leaf_Counter
	-rm -f Aggregate_Votes