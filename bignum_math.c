
/**************************
 * bignum_math.c -- an outline for CLab1
 *
 * orginially written by Andy Exley
 * modified by Emery Mizero
 **************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bignum_math.h"

int* add(int* input1, int* input2, int base);
int* subtract(int* input1, int* input2, int base);
int bignum_length(int* num);
int* compareLessThan(int* input1, int* input2, int base);
int* compareEqualTo(int* input1, int* input2, int base);
int* compareGreaterThan(int* input1, int* input2, int base);

/*
 * Returns true if the given char is a digit from 0 to 9
 */
bool is_digit(char c) {
	return c >= '0' && c <= '9';
}

/*
 * Returns true if lower alphabetic character
 */
bool is_lower_alphabetic(char c) {
	return c >= 'a' && c <= 'z';
}

/*
 * Returns true if upper alphabetic character
 */
bool is_upper_alphabetic(char c) {
	return c >= 'A' && c <= 'Z';
}

/*
 * Convert a string to an integer
 * returns 0 if it cannot be converted.
 */
int string_to_integer(char* input) {
	int result = 0;
	int length = strlen(input);
    int num_digits = length;
	int sign = 1;
	
	int i = 0;
    int factor = 1;

    if (input[0] == '-') {
		num_digits--;
		sign = -1;
    }

	for (i = 0; i < num_digits; i++, length--) {
		if (!is_digit(input[length-1])) {
			return 0;
		}
		if (i > 0) factor*=10;
		result += (input[length-1] - '0') * factor;
	}

    return sign * result;
}

/*
 * Returns true if the given base is valid.
 * that is: integers between 2 and 36
 */
bool valid_base(int base) {
	if(!(base >= 2 && base <= 36)) { 
		return false; 
	}
	return true;
}

/*
 * converts from an array of characters (string) to an array of integers
 */
int* string_to_integer_array(char* str) {
	int* result;
	int i, str_offset = 0;
	int k;
	result = malloc((strlen(str) + 1) * sizeof(int));
	/* If the number is negative, the terminating value is -2 */
	if(str[0] == '-') {
		result[strlen(str) - 1] = -2;
		str_offset = 1;
	/* If the number is positive, the terminating value is -1 */
	} else { 
		result[strlen(str)] = -1;
	}
	for(i = str_offset; str[i] != '\0'; i++) {
		if(is_digit(str[i])) { 
			result[i - str_offset] = str[i] - '0';
		} else if (is_lower_alphabetic(str[i])) {
			result[i - str_offset] = str[i] - 'a' + 10;
		} else if (is_upper_alphabetic(str[i])) {
			result[i - str_offset] = str[i] - 'A' + 10;
		} else {
			printf("I don't know how got to this point!\n");
		}
	}

	for (k=0; k<bignum_length(result); k++) {
		printf("%d\n", result[k]);
	}

	return result;
}

/*
 * TODO
 * Returns true if the given string (char array) is a valid input,
 * that is: digits 0-9, letters A-Z, a-z
 * and it should not violate the given base and should not handle negative numbers
 */
bool valid_input(char* input, int base) {
	
	/* Checks if the base is valid */
	if(!valid_base(base)) {
		printf("Error: That is not a valid base.\n");
		return false;
	}
	
	/* Checks that the input string only contains valid characters */
	int i, str_offset = 0;

	/* Checks to see if the number is negative */
	/* Only checks for negative in the first slot as that is the only acceptable spot */
	if (input[0] == '-') {
		str_offset = 1;
	}

	for(i = str_offset; input[i] != '\0'; i++) {
		if(!(is_digit(input[i]) || is_lower_alphabetic(input[i]) || is_upper_alphabetic(input[i]))) {
			printf("Error: That input does not contain valid characters.\n");
			return false;
		}
	}

	char* copyInput = input;
	
	/* Checks to see that the base isn't violated. I.E. base 10 cannot contain letters */
	int* integerArray;
	integerArray = string_to_integer_array(copyInput);
	
	for(i = 0; integerArray[i] != -1 && integerArray[i] != -2; i++) {
		if(integerArray[i] >= base) {
			printf("Error: That input violates the given base.\n");
			return false;
		}
	}

	return true;
}

/*
 * finds the length of a bignum... 
 * simply traverses the bignum until a negative number is found.
 */
int bignum_length(int* num) {
	int len = 0;
	while(num[len] >= 0) { len++; }
	return len;
}

/*
 * TODO
 * Prints out a bignum using digits and lower-case characters
 * Current behavior: prints integers
 * Expected behavior: prints characters
 */
void bignum_print(int* num) {
	int i;
	if(num == NULL) { return; }

	/* Handle negative numbers as you want */
	/* negative numbers are represented by ending with -2 */
	i = bignum_length(num);
	if(num[i] == -2) {
		printf("-");
	}

	/* Then, print each digit */
	for(i = 0; num[i] >= 0; i++) {
		char printChar;
		int asciiOffset;

		if(num[i] >= 10) {
			asciiOffset = num[i] - 10;
			printChar = 'a' + asciiOffset;
		} else {
			printChar = num[i] + '0';		}

		printf("%c", printChar);
	}
	printf("\n");
}

/*
 *	Helper for reversing the result that we built backward.
 *  see add(...) below
 */
void reverse(int* num) {
	int i, len = bignum_length(num);
	for(i = 0; i < len/2; i++) {
		int temp = num[i];
		num[i] = num[len-i-1];
		num[len-i-1] = temp;
	}
}

/*
 * Helper function for checking if an input is negative
 *
 */

bool isNegative(int* input) {
	if(input[bignum_length(input)] == -2) {
		return true;
	}
	return false;
}

/*
 * Compare operators
 * Check less than by:
 * Check for equivalence,
 * if not, go through element by element.
 * The first digit with the smaller element will be smaller.
 * If one of the numbers is negative, the answer will invert.
 */

int* compareLessThan(int* input1, int* input2, int base) {
	int len1 = bignum_length(input1);
	int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 1;
	int* result = malloc (sizeof(int) * resultlength);
	int r = 0;
	int carry = 0;
	int sign = input1[len1];
	int num1, num2;

	result[0] = 1;


	/* Check if both numbers are + or -
	 * If they are of differet signs, they are not equal.
	 */
	if(compareEqualTo(input1, input2, base)[0] == 1) {
		result[0] = 0;
		result[1] = -1;
		return result;
	}

	len1--;
	len2--;

	result = compareGreaterThan(input1, input2, base);
	if(result[0] == 0) {
			result[0] = 1;
		} else {
			result[0] = 0;
		}
	result[1] = -1;
	return result;

}

/* Check greater than by:
	Check if equal,
	if not, return the opposite of less than
*/
int* compareGreaterThan(int* input1, int* input2, int base) {
	int len1 = bignum_length(input1);
	int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 1;
	int* result = malloc (sizeof(int) * resultlength);
	int r = 0;
	int carry = 0;
	int sign = input1[len1];
	int num1, num2;

	result[0] = 1;


	/* Check if both numbers are + or -
	 * If they are of differet signs, they are not equal.
	 */
	if(compareEqualTo(input1, input2, base)[0] == 1) {
		result[0] = 0;
		result[1] = -1;
		return result;
	}

	if(isNegative(input1) && !isNegative(input2)) {
		result[0] = 0;
		result[1] = -1;
		return result;
	}

	if(!isNegative(input1) && isNegative(input2)) {
		result[0] = 1;
		result[1] = -1;
		return result;
	}

	len1--;
	len2--;

	/* Iterate through each number starting from the end */
	/* If one number is more digits than the other, substitute 0 */

	while (len1 >= 0 || len2 >= 0) {
        if (len1 >= 0) {
            num1 = input1[len1];
        } else {
            num1 = 0;
        }

        if (len2 >= 0) {
            num2 = input2[len2];
        } else {
            num2 = 0;
        }

        if(num1 > num2) {
        	result[r] = 1;
        } else {
        	result[r] = 0;
        }

		len1--;
		len2--;
		r++;
		}

	result[r] = -1;
	reverse(result);
	if(isNegative(input1) && isNegative(input2)) {
		if(result[0] == 0) {
			result[0] = 1;
		} else {
			result[0] = 0;
		}
	}
	result[1] = -1;
	return result; 


}


/*
 * Check equivalence by essentially going through
 * element by element. If elements don't match, return 0.
 */

int* compareEqualTo(int* input1, int* input2, int base) {
	int len1 = bignum_length(input1);
	int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 1;
	int* result = malloc (sizeof(int) * resultlength);
	int r = 0;
	int carry = 0;
	int sign = input1[len1];
	int num1, num2;

	result[0] = 1;


	/* Check if both numbers are + or -
	 * If they are of differet signs, they are not equal.
	 */
	if(input1[len1] != input2[len2]) {
		result[0] = 0;
		result[1] = -1;
		return result;
	}

	len1--;
	len2--;

	/* Iterate through each number starting from the end */
	/* If one number is more digits than the other, substitute 0 */

	while (len1 >= 0 || len2 >= 0) {
        if (len1 >= 0) {
            num1 = input1[len1];
        } else {
            num1 = 0;
        }

        if (len2 >= 0) {
            num2 = input2[len2];
        } else {
            num2 = 0;
        }

        if(num1 != num2) {
        	result[0] = 0;
        	result[1] = -1;
        	return result;
        }
		len1--;
		len2--;
		r++;
		}

	result[1] = -1;
	reverse(result);
	return result;

}


/*
 * used to add two numbers with the same sign
 * GIVEN FOR GUIDANCE
 */
int* add(int* input1, int* input2, int base) {
	int len1 = bignum_length(input1);
	int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 2;
	int* result = (int*) malloc (sizeof(int) * resultlength);
	int r = 0;
	int carry = 0;
	int sign;
    int num1, num2;

    if(isNegative(input1) && isNegative(input2)) {
    	sign = -2;
    }

    if(!isNegative(input1) && !isNegative(input2)) {
    	sign = -1;
    }

   	if(isNegative(input1) && !isNegative(input2)) {
   		input1[len1] = -1;
   		result = subtract(input2, input1, base);
   		return result;
   	}

  	if(!isNegative(input1) && isNegative(input2)) {
   		input2[len2] = -1;
   		result = subtract(input1, input2, base);
   		return result;
   	}

	len1--;
	len2--;

	while (len1 >= 0 || len2 >= 0) {
        if (len1 >= 0) {
            num1 = input1[len1];
        } else {
            num1 = 0;
        }

        if (len2 >= 0) {
            num2 = input2[len2];
        } else {
            num2 = 0;
        }
		result[r] = (num1 + num2 + carry) % base;
		carry = (num1 + num2 + carry) / base;
		len1--;
		len2--;
		r++;
	}
	if (carry > 0) {
        result[r] = carry; 
        r++; 
    }
	result[r] = sign;
	reverse(result);

	return result;
}


/*
 * used to subtract two numbers with the same sign
 * 
 */
int* subtract(int* input1, int* input2, int base) {
	int len1 = bignum_length(input1);
	int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 2;
	int* result = (int*) malloc (sizeof(int) + 1 * resultlength);
	int r = 0;
	int carry = 0;
	int sign = -1;
	int num1, num2;
	int i;
	int k;
	int storage;

	/*

	if(len2 > len1) {
		for(i = 0; i < len2; i++) {
			input2[i] = input2[i - 1];
		} 
		for(i = 0; i < len2-len1; i++) {
			input2[i] = 0;
		}
	}

	/* Compare input1 and input2
	 * If Input1 < Input2, swap their positions as inputs
	 * Swapping their positions then toggles the sign to be opposite of what it was.
	 */

	int* compare = compareLessThan(input1, input2, base);
	if(compare[0] == 1) {
		
		int temp;
		temp = input1;
		input1 = input2;
		input2 = temp;
		printf("Results switched!\n");
	}

    if(isNegative(input1) && isNegative(input2)) {
    	sign = -2;
    	if (compare[0]) { sign = -1;}
    }

    if(!isNegative(input1) && !isNegative(input2)) {
    	sign = -1;
    	if (compare[0]) { sign = -2;}
    }

   	if(isNegative(input1) && !isNegative(input2)) {
   		input1[bignum_length(input1)] = -1;
   		result = add(input2, input1, base);
   		result[bignum_length(result)] = -2;
   		if (compare[0]) { result[bignum_length(result)] = -1;};
   		return result;
   	}

  	if(!isNegative(input1) && isNegative(input2)) {
   		input2[len2] = -1;
   		result = add(input2, input1, base);
   		result[bignum_length(result)] = -1;
    	if (compare[0]) { result[bignum_length(result)] = -2;}
   		
   		return result;
   	}

	len1--;
	len2--;


	/* Iterate through each number starting from the end */
	/* If one number is more digits than the other, substitute 0 */

	while (len1 >= 0 || len2 >= 0) {
        if (len1 >= 0) {
            num1 = input1[len1];
        } else {
            num1 = 0;
        }

        if (len2 >= 0) {
            num2 = input2[len2];
        } else {
            num2 = 0;
        }

        /* Subtract the numbers. If the result is negative, add the base back in and
        flag a carry (-1) to be subtracted from the next place */

       	result[r] = (carry + num1 - num2);
      		if(result[r]<0) {
      			carry = -1;
      			result[r] = result[r] + base;
      		} else {
      			carry = 0;
      		}
		len1--;
		len2--;
		r++;
		}

	result[r] = sign;

	reverse(result);

	return(result);

}



/*
 * TODO
 * This function is where you will write the code that performs the heavy lifting, 
 * actually performing the calculations on input1 and input2.
 * Return your result as an array of integers.
 * HINT: For better code structure, use helper functions.
 */
int* perform_math(int* input1, int* input2, char op, int base) {

	/* 
	 * this code initializes result to be large enough to hold all necessary digits.
	 * if you don't use all of its digits, just put a -1 at the end of the number.
	 * you may omit this result array and create your own.
     */

    int len1 = bignum_length(input1);
    int len2 = bignum_length(input2);
	int resultlength = ((len1 > len2)? len1 : len2) + 1;
	int* result = (int*) malloc (sizeof(int) * resultlength);
	int sign = -1;

	if(op == '+') {
		return add(input1, input2, base);
	} else if(op == '-') {
		return subtract(input1, input2, base);
	} else if(op == '>') {
		return compareGreaterThan(input1, input2, base);
	} else if(op == '<') {
		return compareLessThan(input1, input2, base);
	} else if(op == '=') {
		return compareEqualTo(input1, input2, base);
	}
/* Write your logic for subtraction and comparison here*/
	return result;
}

/*
 * Print to "stderr" and exit program
 */
void print_usage(char* name) {
	fprintf(stderr, "----------------------------------------------------\n");
	fprintf(stderr, "Usage: %s base input1 operation input2\n", name);
	fprintf(stderr, "base must be number between 2 and 36, inclusive\n");
	fprintf(stderr, "input1 and input2 are arbitrary-length integers\n");
	fprintf(stderr, "Two operations are allowed '+' and '-'\n");
	fprintf(stderr, "----------------------------------------------------\n");
	exit(1);
}


/*
 * MAIN: Run the program and tests your functions.
 * sample command: ./bignum 4 12 + 13
 * Result: 31
 */
int main(int argc, char** argv) {

	int input_base;

    int* input1;
    int* input2;
    int* result;

	if(argc != 5) { 
		print_usage(argv[0]); 
	}

	input_base = string_to_integer(argv[1]);

	if(!valid_base(input_base)) { 
		fprintf(stderr, "Invalid base: %s\n", argv[1]);
		print_usage(argv[0]);
	}
	

	if(!valid_input(argv[2], input_base)) { 
		fprintf(stderr, "Invalid input1: %s\n", argv[2]);
		print_usage(argv[0]);
	}

	if(!valid_input(argv[4], input_base)) { 
		fprintf(stderr, "Invalid input2: %s\n", argv[4]);
		print_usage(argv[0]);
	}

    char op = argv[3][0];
	if(op != '-' && op != '+' && op != '<' && op != '>' && op != '=') {
		fprintf(stderr, "Invalid operation: %s\n", argv[3]);
		print_usage(argv[0]);
	}

	input1 = string_to_integer_array(argv[2]);
    input2 = string_to_integer_array(argv[4]);

    result = perform_math(input1, input2, argv[3][0], input_base);

    printf("Result: ");
    bignum_print(result);
	printf("\n");

	exit(0);
}
