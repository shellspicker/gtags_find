#!/bin/bash
# use with preprocess by gtags, find func's caller func continuously.

# arg parse: func_pattern
if [ $# -ne 1 ]; then
	echo "input query func pattern"
	exit -1
fi

# arg maybe regex.
pattern=$1
call_path=

# find caller pos for all matching patterns func.
preprocess()
{
	ret=$(global -xr $pattern |
		awk '{printf("%s %d %s\n", $1, $2, $3)}')

	query_name_list=($(echo "$ret" | cut -d' ' -f1))
	line_list=($(echo "$ret" | cut -d' ' -f2))
	file_list=($(echo "$ret" | cut -d' ' -f3))
	query_line_total=${#line_list[*]}
}

# arg: line, file
# find caller func by input line and file.
func_name()
{
	local line func
	local ret
	local l r i n

	# special: judge if arg2(file) is not *.c
	echo "$2" | grep -q '.*\.c'
	if [ $? -ne 0 ]; then
		return 255
	fi
	# show all func name in file.
	ret=$(global -f $2 |
		awk '{printf("%s %d\n", $1, $2)}')

	func=($(echo "$ret" | cut -d' ' -f1))
	line=($(echo "$ret" | cut -d' ' -f2))
	n=${#line[*]}

	for ((i = 0; i < n; ++i)); do
		l=${line[$i]}
		# find first line of } from { line.
		r=$(sed -n "$l"',${/^\}/{=;q}}' $2)
		# in range is caller func, there is only one.
		if (( l <= $1 && $1 <= r )); then
			echo "${func[$i]} $2"
			return 0
		fi
	done
}

# once query
query_main()
{
	local tmp line_i

	preprocess
	line_i=0
	# query pattern may be regex, so there may be many query string.
	for ((xl = 0; xl < $query_line_total; ++xl)); do
		tmp=$(func_name ${line_list[$xl]} ${file_list[$xl]})
		if ((${#tmp} != 0)); then
			((line_i++))
			tmp="$tmp ${line_list[$xl]}"
			printf '%3i %40s %40s %20s %5i\n' $line_i ${query_name_list[$xl]} $tmp
		fi
	done
}

# just sort output from once query.
once_output()
{
	local ret

	ret=$(query_main | sort -n -k1,5 -u)
	echo "$ret"
}

# multiple query with key interaction.
multi_query()
{
	local reftable last_pattern
	local down up up_file up_line first first_pattern
	local qnum

	first=1
	first_pattern=
	for ((;;)); do
		if [ -z "$pattern" ]; then
			# quit.
			echo 'QAQ: quit.'
			echo "$call_path" | less
			return 0
		elif [ "$last_pattern" = "$pattern" ]; then
			# just show last output.
			echo "$reftable" | less
		else
			# new query.
			reftable=$(once_output)
			echo "$reftable" | less
		fi
		last_pattern="$pattern"
		pattern=""
		# input branch for next query.
		read qnum
		echo $qnum | grep '[0-9]' -q
		if [ $? -eq 0 ]; then
			# new query pattern.
			# choose the caller func from current output
			# as next querry pattern.
			# now can recording the call path.
			down=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $2}')
			pattern=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $3}')
			up_file=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $4}')
			up_line=$(echo "$reftable" | sed -n ${qnum}p | awk '{print $5}')
			up="$up_file:$up_line:$pattern"
			if ((first == 1)); then
				first=0
				first_pattern=$down
				call_path=$(echo -en "$first_pattern\n$up")
			else
				call_path=$(echo -en "$call_path\n$up")
			fi
		elif [[ "$qnum" = 'q' || "$qnum" = 'Q' ]]; then
			pattern=""
		else
			pattern=$last_pattern
		fi
	done
}

# choose once or multi query.
#once_output
multi_query

# this just for test(debug).
#query_main
