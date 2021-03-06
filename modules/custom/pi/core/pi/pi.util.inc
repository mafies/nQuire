<?php
/**
 * @file nQuire general utility functions
 */

/**
 * Check if the given $node is a node of type given
 * @param $possible_node
 * @param string $desired_type
 * @return bool true if possible node is a node of desired type false if not 
 */
function pi_check_is_node_of_type($possible_node, $desired_type)
{
  $check = false;
  if(is_object($possible_node))
  {
    if($possible_node->type==$desired_type)
    {
      $check= true;
    }
  }
  return $check;
}

/**
 * Check if a node exists and is of the given type
 * @param uint $possible_nid
 * @param string $desired_type
 * @return bool true if possible node exists and is a node of desired type false if not 
 */
function pi_check_node_exists_and_is_of_type($possible_nid, $desired_type)
{
  return (is_numeric($possible_nid) ? pi_check_is_node_of_type(node_load($possible_nid), $desired_type) : false);
}


/**
 * Convert a result constant from drupal_write_record to a string for debug purposes
 * @param int $result Result variable as a defined consant from drupal_write_record, as defined in common.int
 * @return string A string representing the result status, or the number if it's an undefined value
 */
function _pi_write_record_result_string($result)
{
	$result_string = "";
	switch($result)
	{
		case(SAVED_NEW):
			$result_string = "SAVED_NEW";
			break;
		case SAVED_UPDATED:
			$result_string = "SAVED_UPDATED";
			break;
		case SAVED_UPDAED:
			$result_string = "SAVED_DELETED";
			break;
		default:
			$result_string = "$result";
	}
	return $result_string;
}



/**
 * This is the validation function to make sure that pi_phase.inquiry_nid is a valid inquiry
 * @param unknown_type $element
 * @param unknown_type $form_state
 */
function pi_util_inquiry_nid_validate($element, $form_state)
{
  if($element['#type']!= 'hidden' && !in_array($element['#value'], $element['#allowed_inquiry_nids']))
  {
    form_error($element, t('The selected inquiry is not valid, please choose an existing inquiry.'));
  }
}


/**
 * This is the validation function to make sure that pi_phase.inquiry_nid is a valid inquiry
 * @param unknown_type $element
 * @param unknown_type $form_state
 */
function pi_util_phase_nid_validate($element, $form_state)
{
	if($element['#type']!= 'hidden' && !in_array($element['#value'], $element['#allowed_phase_nids']))
	{
		form_error($element, t('The selected phase is not valid, please choose an existing phase from an inquiry.'));
	}
}

/**
 * This compares the ['#weight'] elements in two arrays
 * @param array $x
 * @param array $y
 * @return bool result of $x['#weight'] > $y['#weight']
 */
function pi_util_weight_compare($x, $y) {
	$xw = (int) $x['#weight'];
	$yw = (int) $y['#weight'];
	return $xw > $yw;
}

/**
 *
 * Sort an array of arrays by one of their elements - e.g. a list of set result rows to be
 * sorted by a column.  Key and value associations are maintained.
 * @param $array Array of arrays to be sorted
 * @param $element the name of the array key to sort by
 */
function pi_sort_arrays_by_element(&$array, $element)
{
	$return_array = array();
	$element_array = array();
	foreach($array as $key => $value)
	{
		$element_array[$key] = $value[$element];
	}
	asort($element_array);
	foreach($element_array as $key)
	{
		$return_array[$key] = $array[$key];
	}
	$array = $return_array;
}


/**
 * This is a utility function to merge two content item lists which have '#weight' elements
 * e.g. item[x]['#weight'].  Weight elements in the second list are ignored and this list is
 * appended to the ordrered items in their default order.
 * If an unsorted item occurrs in the sorted item list, the sorted item weight takes precidence
 * Weight attributes are renumbered incrementally from 0
 * @param array(array) $sorted_items content item list with '#weight' elements
 * @param $unsorted_items  $sorted_items content item list - weight ignored
 * @return array of items sorted by weight
 */
function pi_merge_content_item_lists($sorted_items, $unsorted_items)
{
	$return_data = array();
	$weight_data = array();
	if ($sorted_items)
	{
		foreach($sorted_items as $key => $sorted_item)
		{
			$weight_data[$key] =  $sorted_item['#weight'];
		}
	}
	asort($weight_data);
	foreach(array_keys($weight_data) as $ordered_item_key)
	{
		$sorted_items[$ordered_item_key]['#weight'] = count($return_data);
		$return_data[$ordered_item_key] = $sorted_items[$ordered_item_key];
	}
	if( is_array( $unsorted_items ) )
	{
		foreach($unsorted_items as $unsorted_item_key => $unsorted_item)
		{
			if(!array_key_exists($unsorted_item_key, $return_data))
			{
				$unsorted_item['#weight'] = count($return_data);
				$return_data[$unsorted_item_key] = $unsorted_item;
			}
		}
	}
	return $return_data;
}

/**
 * This is a function to sort items by first array index then then by #weight property
 * The status order is determined by $group_order, which will set the order for the groups
 * the #weight parameter of the items will be reset to a sequential series starting from 0
 * any groups that are not listed are added to the end of the list in order of occurrence
 * @param array $group_order array of indexes to define what order the grouped items will be sorted, e.g. key, selected, none
 * @param $grouped_items array of arrays of items with #weight parameters.
 * 	e.g. array(
 * 		'key' =>
 * 			array(
 * 				'123'=> array('#weight' => 12),
 * 				'321'=>array('#weight' => 32),
 * 			),
 * 		'selected' => etc
 * @param string $weight_index this will usually be #weight for display items
 * 	but for form data it could be anything, e.g. 'weight' is often used
 * @return array Flattened array of items from $grouped_items with a sequential #weight value.
 */
function pi_sort_grouped_item_list($group_order, $grouped_items, $weight_index)
{
	$item_ids = array();
	$item_groups = array();
	$item_weights = array();
	$items = array();
	
	//In order to avoid error message from foreach in case $grouped_itemes is NULL
	$grouped_items = is_array( $grouped_items ) ? $grouped_items : array();
	
	foreach($grouped_items as $group_name => $group)
	{
		if( is_array($item_ids) )
		{
			if( !array_key_exists($group_name, $item_ids) ||
			( array_key_exists($group_name, $item_ids) && !is_array($item_ids[$group_name]) ) )
			{
				$item_ids[$group_name] = array();
			}
		}

		if( is_array($item_weights) )
		{
			if( !array_key_exists($group_name, $item_weights) ||
			( array_key_exists($group_name, $item_weights) && !is_array($item_weights[$group_name]) ) )
			{
				$item_weights[$group_name] = array();
			}
		}

		foreach($group as $item_id => $item)
		{
			$item_ids[$group_name][] = $item_id;
			$item_weights[$group_name][] = $item[$weight_index];
			$items[$item_id] = $item;
		}
	}

	// If new groups appear that aren't in the default list, append them to the end of the list:
	foreach(array_keys($item_ids) as $other_group)
	{
		if(!in_array($other_group, $group_order))
		{
			$group_order[] = $other_group;
		}
	}
	$sorted_items = array();
	foreach($group_order as $group)
	{
		if(array_key_exists($group, $item_ids)!=NULL)
		{
			$sort_weights = $item_weights[$group];
			$sort_ids = $item_ids[$group];
			array_multisort($sort_weights, $sort_ids);
			foreach($sort_ids as $item_id)
			{
				$items[$item_id][$weight_index] = count($sorted_items);
				$sorted_items[$item_id] = $items[$item_id];
			}
		}
	}
	return $sorted_items;
}

/**
 * This function takes a flattened array of items and groups them into arrays accouting to
 * the values of group_field
 * @param array or array $items items to sort
 * @param string $group_field the field of the array by which to group items
 * @return a nested array, first index is the value of $items['group_field'] and the second index the current
 * key of $item.
 * array('group_field1' => array ('id'=>array(value=>'value, group_field=>'group_field1', .), .. )...)
 */
function pi_create_grouped_item_list($items, $group_field)
{
	$return_array=array();
	foreach($items as $key => $item)
	{
		$group_key = $item[$group_field];
		if(!array_key_exists($group_key, $return_array))
		{
			$return_array[$group_key]=array();
		}
		$return_array[$group_key][$key] = $item;
	}
	return $return_array;
}

/**
 *
 * This does the opposite of pi_create_grouped_item_list and takes all the elements of $item_list
 * and combines them into one array
 * n.b. if items in the sub-arrays have dupliucate keys, the rturn element will be overwritten by the value of the
 * later sub-list element.
 * @param array(array(),array(),array()) $item_list
 * @return array of contents of the above array elements in a single array
 */
function pi_flatten_grouped_item_list($item_list)
{
	$list = array();
	$all_defined_groups = array();
	foreach(array_keys($item_list) as $region)
	{
		foreach($item_list[$region] as $key => $item)
		{
			$list[$key] = $item;
		}
	}
	return $list;
}


/**
 *
 * This ensures that there is only one unique row in $table with $id_field equal to $id
 *  if there are no rows, it returns zero, otherwise it returns one.
 *  If more than one row exists then the lowest $id is kept and the rest are deleted.
 * @param unknown_type $table
 * @param unknown_type $id_field
 * @param int $id
 * @return bool true if one row is found false if no row exists;
 */
function pi_db_ensure_unique($table, $id_field, $id)
{
	$one_row = false;
	// See how many rows there are are
	$query = "SELECT COUNT(%s) FROM {%s} WHERE %s = %d";
	$result = db_query($query, $id_field, $table, $id_field, $id);
	$count = db_result($result);
	// Check if the group type already exists and leave only one definition left
	if($count>1)
	{
		// if the id has any duplicates remove them...
		$query = "DELETE FROM {%s} WHERE %s = %d  LIMIT %d";
		db_query($query, $table, $id_field, $id, $count-1);
		$one_row = true;
	}
	else
	{
		$one_row = $count;
	}
	return $one_row;
}

/**
 *
 * Return an array indexed by nid with value the title of the node, or NULL
 * if the node is not found
 * @param array uint $nids Array of node ids
 * @return  array('nid' => 'title', ...)
 */
function pi_util_get_names_from_nids($nids)
{
	$return_data = array();
	if(is_array($nids))
	{
		foreach($nids as $nid)
		{
			$n = node_load($nid);
			if($n!=NULL)
			{
				$return_data[$nid] = $n->title;
			}
			else
			{
				$return_data[$nid] = NULL;
			}
		}
	}
	return $return_data;
}


/**
 * Shorten text to length given, trimmign whole words and add postfix text to the end if it's shortened
 * @param $text
 * @param $length
 * @param $postfix
 */
function pi_util_trim_text($text, $length=250, $postfix="...") {
	$return_text = $text;
	if(strlen($text) > $length) {
		$short_body = substr($text, 0, $length);
		$last_space = strrpos($short_body, " ");
		$short_body = substr($short_body, 0, $last_space);
		$return_text = $short_body . $postfix;
	}
	return $return_text;
}


/**
 * This function updates the title of the node in the node and node_revision table
 * @param $node
 * @param $title
 */
function pi_util_set_node_title($node, $title=NULL) {
	if ($title === NULL)
		$title = $node->body;
	$node->title = pi_util_trim_text(check_plain($title));
	node_save($node);
}


/**
 * Return an array of the drupal path elements in parts or null if there's a problem
 */
function pi_util_get_drupal_path_parts()
{
	$drupal_path_parts = NULL; 
	//Strip the request url of the preceeding drupal stuff base_url and ?q=/
	// so that it's possible to work out whether the page request was from /inquiry/%inquiry/phase/add
	// or from /node/%node/edit
	$drupal_path = str_replace(base_path(), '', request_uri());
	$drupal_path = str_replace('?q=', '', $drupal_path);
	// Split the string into parts
	$drupal_path_parts = explode('/', $drupal_path);
	return $drupal_path_parts;
}


/**
 * 
 * See if a url matches a given pattern, in the array specified. Each element in the array is compared with path parts
 * and if the element is NULL then it's considered a wildcard, size of the arrays is the minimum number of parts in the path, no maximum length can
 * be specified here
 * e.g. array("activity", NULL, "edit", NULL) would match the path
 * /activity/ * /edit/ *   
 *  URL parameters, and the drupal installation folder are ignored
 * @param String $url
 * @param array $matches
 */
function pi_util_match_url($matches = array())
{
	// Initially set it to true since we are only looking for one mismatch
	$return_value = true;
	// Split the string into parts 
	$path_parts =pi_util_get_drupal_path_parts();
	//See if there's the right number of path parts...
	if(count($path_parts)>=count($matches))
	{
		// Now check all the parts for vaildity... 
		$count = 0;
		foreach($matches as $match)
		{
			//pi_debug_message("compare " . $match . " with " . $path_parts[$count]);
			if($match!=NULL)
				if(strcasecmp($path_parts[$count], $match)!==0)
					$return_value = false;
			$count++;
		}
	}
	else {
		$return_value = false;
	}
	return $return_value;
}

/**
 * Return the path part with the given index (see pi_util_get_drupal_path_parts)
 * @param uint $index Starting at 0
 */
function pi_util_get_path_element($index)
{
	$parts = pi_util_get_drupal_path_parts();
	return $parts[$index];
}

/*pre-php 5.3 array_replace compatability fix*/
/*this allows code to work for Paul*/
if (!function_exists('array_replace'))
{
	function array_replace( array &$array, array &$array1, $filterEmpty=false )
	{
		$args = func_get_args();
		$count = func_num_args()-1;

		for ($i = 0; $i < $count; ++$i) {
			if (is_array($args[$i])) {
				foreach ($args[$i] as $key => $val) {
					if ($filterEmpty && empty($val)) continue;
					$array[$key] = $val;
				}
			}
			else {
				trigger_error(
						__FUNCTION__ . '(): Argument #' . ($i+1) . ' is not an array',
						E_USER_WARNING
				);
				return NULL;
			}
		}

		return $array;
	}
}

?>