/* ============================================================================================================== */
/* == BEGIN FILE ================================================================================================ */
/* ============================================================================================================== */
/*
 ============================================================================
 Name        : taas_aaf.c
 Author      : Matthias Thimm
 Version     : 2.0
 Copyright   : GPL3
 Description : General functions on AAFs for taas solvers.
 ============================================================================
 */

struct AAF{
  /** Mapping internal argument identifiers (ints) to argument names **/
  char** ids2arguments;
  /** Mapping argument names to internal argument identifiers (ints)**/
  GHashTable* arguments2ids;
  /** The number of arguments. */
  int number_of_arguments;
  /** The number of attacks. */
  int number_of_attacks;
  /** Maps arguments to their children */
  GSList** children;
  /** Maps arguments to their parents */
  GSList** parents;
  /** Maps arguments to the number of their number of attackers
     (after computing the grounded extension this equals the number
      of attackers not already out in the grounded extension) */
  int* number_of_attackers;
  /** The initial arguments (unattacked ones) */
  struct BitSet* initial;
  /** Self-attacking arguments */
  struct BitSet* loops;
};

void taas__aaf_destroy(struct AAF* aaf){
  for(int i = 0; i < aaf->number_of_arguments; i++){
		free(aaf->ids2arguments[i]);
    g_slist_free(aaf->children[i]);
    g_slist_free(aaf->parents[i]);
  }
	free(aaf->ids2arguments);
  free(aaf->children);
  free(aaf->parents);
	g_hash_table_destroy(aaf->arguments2ids);
  free(aaf);
}

// Returns TRUE iff i attacks j
int taas__aaf_isAttack(struct AAF* aaf, int i, int j){
  for(GSList* node = aaf->children[i]; node != NULL; node = node->next)
    if(*(int*)node->data == j)
      return TRUE;
  return FALSE;
}
/* ============================================================================================================== */
/* == END FILE ================================================================================================== */
/* ============================================================================================================== */
