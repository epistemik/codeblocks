#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int find_left_child(int index_position_of_parent);

int find_right_child(int index_position_of_parent);

int find_parent(int child_index_position);

void satisfy_condition_for_node(int a[], int node_of_index, int length_of_array);

void satisfy_condition_for_all_nodes(int a[], int number_of_non_leaf_nodes);

int main()
{
    int a[] = {5,5,85,8,6,85,5245,6,81,423,4,3,56,6,5};
    int size = sizeof(a)/sizeof(a[0]);

    printf("%d\n\n\n",find_right_child(0));

    printf("%d\n\n\n", find_parent(7));

    satisfy_condition_for_node(a,1, size);
    satisfy_condition_for_node(a,2, size);

    int nonleaf = 0;

    while(find_left_child(nonleaf) < size)
        nonleaf++;

    int i;
    for(i = 0;i < size;i++)
        printf("%d ", a[i]);

    return 0;
}

int find_left_child(int index_position_of_parent)
{
    int row = 0;
    while((pow(2, row+1)-1) <= index_position_of_parent)
        row++;

    int column = index_position_of_parent;
    int i;
    for(i = row-1; i >= 0; i--)
        column -= pow(2,i);

    return ((pow(2,row+1)-1) + 2*column);
}

int find_right_child(int index_position_of_parent)
{
    return find_left_child(index_position_of_parent) + 1;
}

int find_parent(int child_index_position)
{
    int row = 0;
    while((pow(2, row+1)-1) <= child_index_position)
        row++;

    int column = child_index_position;
    int i;
    for(i = row-1; i >= 0; i--)
        column -= pow(2,i);

    return((pow(2, row-1)-1) + column/2);
}
void satisfy_condition_for_node(int a[], int node_of_index, int length_of_array)
{
    if (a[node_of_index] > a[find_parent(node_of_index)])
    {
        int temp;
      if(node_of_index % 2 == 0)
      {
          if(a[node_of_index-1] > a[node_of_index])
          {
              temp = a[node_of_index-1];
              a[node_of_index-1] =  a[find_parent(node_of_index)];
              a[find_parent(node_of_index)] = temp;
          }
          else
          {
              temp = a[node_of_index];
              a[node_of_index] =  a[find_parent(node_of_index)];
              a[find_parent(node_of_index)] = temp;
          }
      }
      else
      {
          if(a[node_of_index+1] > a[node_of_index])
          {
              temp = a[node_of_index+1];
              a[node_of_index+1] =  a[find_parent(node_of_index)];
              a[find_parent(node_of_index)] = temp;
          }
          else
          {
              temp = a[node_of_index];
              a[node_of_index] =  a[find_parent(node_of_index)];
              a[find_parent(node_of_index)] = temp;
          }
      }

    }
    if(node_of_index + 1 < length_of_array)
        satisfy_condition_for_node(a, node_of_index+1, length_of_array);
}

void satisfy_condition_for_all_nodes(int a[], int number_of_non_leaf_nodes)
{
    int i;
    for (i=0; i < number_of_non_leaf_nodes; i++)
        satisfy_condition_for_node(a, i, sizeof(a)/sizeof(a[0]));
}


