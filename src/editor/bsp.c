#include "editor/bsp.h"
#include <assert.h>

#define BSP_CONT_CAPACITY_NODE_INIT 64
#define BSP_CONT_CAPACITY_LEAF_INIT 64
#define BSP_CONT_CAPACITY_FACE_INIT 128

typedef enum
{
  SPLIT_FRONT = 1,
  SPLIT_BACK = 2,
  SPLIT_BOTH = 3
} split_result_t;


static void bsp_container_init(bsp_tree_t *cont)
{
  assert(cont);

  cont->node_count = 0;
  cont->leaf_count = 0;
  cont->face_count = 0;

  cont->node_capacity = BSP_CONT_CAPACITY_NODE_INIT;
  cont->leaf_capacity = BSP_CONT_CAPACITY_LEAF_INIT;
  cont->face_capacity = BSP_CONT_CAPACITY_FACE_INIT;

  cont->nodes = malloc(sizeof(bsp_node_t) * cont->node_capacity);
  cont->leaves = malloc(sizeof(bsp_leaf_t) * cont->leaf_capacity);
  cont->faces = malloc(sizeof(bsp_face_t) * cont->face_capacity);

  assert(cont->nodes);
  assert(cont->leaves);
  assert(cont->faces);
}

static void bsp_container_reset(bsp_tree_t *cont)
{
  assert(cont);
  cont->node_count = 0;
  cont->node_capacity = 0;

  cont->leaf_count = 0;
  cont->leaf_capacity = 0;

  cont->face_count = 0;
  cont->face_capacity = 0;
}

static void bsp_container_free(bsp_tree_t *cont)
{
  assert(cont);

  bsp_container_reset(cont);

  if (cont->nodes)
    free(cont->nodes);
  if (cont->leaves)
    free(cont->leaves);
  if (cont->faces)
    free(cont->faces);
}

static void bsp_io_writetofile(const char *filepath, bsp_tree_t *cont);

bsp_tree_t *bsp_io_readfromfile(const char *filepath);

static split_result_t split_brush(
  brush_t* in,
  plane_t splane, 
  brush_t** front, 
  brush_t** back
)
{
  if (!in) return;

  brush_t* f = calloc(1, sizeof(brush_t));
  brush_t* b = calloc(1, sizeof(brush_t));

  assert(f);
  assert(b);

  f->contents = in->contents;
  b->contents = in->contents;


}

void BSP_Compile()
{
  bsp_tree_t *container = malloc(sizeof(bsp_tree_t));
  bsp_container_init(container);
}