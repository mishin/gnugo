/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 * This is GNU GO, a Go program. Contact gnugo@gnu.org, or see       *
 * http://www.gnu.org/software/gnugo/ for more information.          *
 *                                                                   *
 * Copyright 1999, 2000, 2001, 2002 by the Free Software Foundation. *
 *                                                                   *
 * This program is free software; you can redistribute it and/or     *
 * modify it under the terms of the GNU General Public License as    *
 * published by the Free Software Foundation - version 2             *
 *                                                                   *
 * This program is distributed in the hope that it will be useful,   *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of    *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the     *
 * GNU General Public License in file COPYING for more details.      *
 *                                                                   *
 * You should have received a copy of the GNU General Public         *
 * License along with this program; if not, write to the Free        *
 * Software Foundation, Inc., 59 Temple Place - Suite 330,           *
 * Boston, MA 02111, USA.                                            *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * sgffile.c
 *
 * This file contains functions that create an SGF file on the fly.
 * There can be only one file open simultaneously.
 *
 * See sgf/sgftree.c for more general handling of SGF trees and file I/O.
 *
 */

#include "gnugo.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "liberty.h"
#include "sgftree.h"
#include "gg_utils.h"

/*
 * Add debug information to a node if user requested it from command
 * line.
 */

void
sgffile_add_debuginfo(SGFNode *node, int value)
{
  int m, n;
  char comment[24];

  if (!outfilename[0])
    return;
  
  for (m = 0; m < board_size; ++m)
    for (n = 0; n < board_size; ++n) {
      if (BOARD(m, n) && (output_flags & OUTPUT_MARKDRAGONS)) {
	switch (dragon[POS(m, n)].crude_status) {
	case DEAD:
	  sgfLabel(node, "X", m, n);
	  break;
	case CRITICAL:
	  sgfLabel(node, "!", m, n);
	  break;
	}
      }
      
      if (potential_moves[m][n] > 0.0 && (output_flags & OUTPUT_MOVEVALUES)) {
	if (potential_moves[m][n] < 1.0)
	  sgfLabel(node, "<1", m, n);
	else
	  sgfLabelInt(node, (int) potential_moves[m][n], m, n);
      }
    }
  
  if (value > 0 && (output_flags & OUTPUT_MOVEVALUES)) {
    sprintf(comment, "Value of move: %d", value);
    sgfAddComment(node, comment);
  }
}


/*
 * Write sgf tree to output file specified with -o option.
 * This can safely be done multiple times.
 */

void
sgffile_output(SGFTree *tree)
{
  if (outfilename[0])
    writesgf(tree->root, outfilename);
}


/* ================================================================ 
 * Dumping of information about a position into an sgftree.
 * Used by sgffile_decideposition, etc.
 * ================================================================ */


/*
 * sgffile_begindump begins storing all moves considered by
 * trymove and tryko in an sgf tree in memory.
 *
 * The caller only has to provide an own SGFTree pointer if he wants
 * to do something more with the tree than writing it to file as done
 * by sgffile_enddump().
 */

void
sgffile_begindump(SGFTree *tree)
{
  static SGFTree local_tree;
  gg_assert(sgf_dumptree == NULL);

  if (tree == NULL)
    sgf_dumptree = &local_tree;
  else 
    sgf_dumptree = tree;
  
  sgftree_clear(sgf_dumptree);
  sgftreeCreateHeaderNode(sgf_dumptree, board_size, 0.0);
  sgffile_printboard(sgf_dumptree);
}


/*
 * sgffile_enddump ends the dump and writes the sgf tree to file.
 */

void 
sgffile_enddump(const char *filename)
{
  /* check if we have a valid filename */
  if (filename && *filename)
    writesgf(sgf_dumptree->root, filename);
  sgf_dumptree = NULL;
}


/*
 * sgffile_loadandprint adds the current board position as well as
 * information about who is to play and illegal moves to the tree.
 */

void
sgffile_loadandprint(SGFTree *tree, int next)
{
  int m, n;
  char pos[3];
  
  sgffile_printboard(tree);
  
  if (next == EMPTY)
    return;

  sgfAddProperty(tree->lastnode, "PL", (next == WHITE ? "W" : "B"));

  for (m = 0; m < board_size; ++m)
    for (n = 0; n < board_size; ++n)
      if (BOARD(m, n) == EMPTY && !is_legal(POS(m, n), next)) {
       gg_snprintf(pos, 3, "%c%c", 'a' + n, 'a' + m);
       sgfAddProperty(tree->lastnode, "IL", pos);
      }
}


/*
 * sgffile_printboard adds the current board position to the tree.
 */

void
sgffile_printboard(SGFTree *tree)
{
  int i, j;
  SGFNode *node;
  
  gg_assert(tree);
  node = sgfAddChild(tree->lastnode);
  
  /* Write the white stones to the file. */
  for (i = 0; i < board_size; i++) {
    for (j = 0; j < board_size; j++) {
      if (BOARD(i, j) == WHITE)
	sgfAddStone(node, WHITE, i, j);
    }
  }

  /* Write the black stones to the file. */
  for (i = 0; i < board_size; i++) {
    for (j = 0; j < board_size; j++) {
      if (BOARD(i, j) == BLACK)
	sgfAddStone(node, BLACK, i, j);
    }
  }

  sgftreeSetLastNode(tree, node);
}


void
sgffile_recordboard(SGFNode *node)
{
  int i, j;

  if (node)
    for (i = 0; i < board_size; i++)
      for (j = 0; j < board_size; j++)
        if (BOARD(i, j) == BLACK)
          sgfAddStone(node, BLACK, i, j);
}


/*
 * Local Variables:
 * tab-width: 8
 * c-basic-offset: 2
 * End:
 */
