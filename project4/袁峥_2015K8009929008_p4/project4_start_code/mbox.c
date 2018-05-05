#include "common.h"
#include "mbox.h"
#include "sync.h"
#include "scheduler.h"


typedef struct
{
	/* TODO */
	node_t    msg_node;
	char      msg[MAX_MESSAGE_LENGTH];
	uint32_t  msg_len;
} Message;

typedef struct
{
	/* TODO */
	uint32_t      msg_num;
	semaphore_t   full,empty; 
	lock_t        lock;
	char          name[MBOX_NAME_LENGTH];
	mbox_t        mid;
	Message       msgs[MAX_MBOX_LENGTH];
	uint32_t      user_num;
	bool_t        used;
	node_t        msg_queue;
} MessageBox;

static MessageBox MessageBoxen[MAX_MBOXEN];
lock_t BoxLock;


/* Perform any system-startup
 * initialization for the message
 * boxes.
 */
void init_mbox(void)
{
	/* TODO */
	int i,j;
	for (i=0; i<MAX_MBOXEN; i++){
		MessageBoxen[i].mid = i;
		MessageBoxen[i].msg_num = 0;
		MessageBoxen[i].user_num = 0;
		MessageBoxen[i].used = FALSE;
		lock_init(&MessageBoxen[i].lock);
		semaphore_init(&MessageBoxen[i].full,MAX_MBOX_LENGTH);
		semaphore_init(&MessageBoxen[i].empty,0);
		queue_init(&MessageBoxen[i].msg_queue);
		for (j=0; j>MAX_MBOX_LENGTH; j++){
			MessageBoxen[i].msgs[j].msg_len = 0;
			MessageBoxen[i].msgs[j].msg[0] = 0;
		}
	}
	lock_init(&BoxLock);
}


/* Opens the mailbox named 'name', or
 * creates a new message box if it
 * doesn't already exist.
 * A message box is a bounded buffer
 * which holds up to MAX_MBOX_LENGTH items.
 * If it fails because the message
 * box table is full, it will return -1.
 * Otherwise, it returns a message box
 * id.
 */
mbox_t do_mbox_open(const char *name)
{
	/* TODO */
	int i, first_unused = -1, len;
	lock_acquire(&BoxLock);
	for (i=0; i<MAX_MBOXEN; i++){
		if (MessageBoxen[i].used ==TRUE && same_string(name,MessageBoxen[i].name)){
			MessageBoxen[i].user_num++;
			lock_release(&BoxLock);
			return MessageBoxen[i].mid;
		}
		else if (MessageBoxen[i].used == FALSE && first_unused == -1)
			first_unused = i;
	}
	if (first_unused == -1){
		lock_release(&BoxLock);
		return -1;
	}
	MessageBoxen[first_unused].used = TRUE;
	len = strlen(name);
	ASSERT(len <= MBOX_NAME_LENGTH);

	for (i=0; i<len; i++)
		MessageBoxen[first_unused].name[i] = name[i];
	MessageBoxen[first_unused].name[len] = 0;
	MessageBoxen[first_unused].user_num = 1;
	lock_release(&BoxLock);
	return MessageBoxen[first_unused].mid;
}


/* Closes a message box
 */
void do_mbox_close(mbox_t mbox)
{
	/* TODO */
	int i;
	lock_acquire(&BoxLock);
	MessageBoxen[mbox].user_num--;
	if (!MessageBoxen[mbox].user_num){
		MessageBoxen[mbox].msg_num = 0;
		MessageBoxen[mbox].user_num = 0;
		MessageBoxen[mbox].used = FALSE;
		MessageBoxen[mbox].name[0] = 0;
		for (i=0; i>MAX_MBOX_LENGTH; i++){
			MessageBoxen[mbox].msgs[i].msg_len = 0;
			MessageBoxen[mbox].msgs[i].msg[0] = 0;
		}
	}
	lock_release(&BoxLock);
}


/* Determine if the given
 * message box is full.
 * Equivalently, determine
 * if sending to this mbox
 * would cause a process
 * to block.
 */
int do_mbox_is_full(mbox_t mbox)
{
	/* TODO */
	return (MessageBoxen[mbox].msg_num == MAX_MBOX_LENGTH);
}


/* Enqueues a message onto
 * a message box.  If the
 * message box is full, the
 * process will block until
 * it can add the item.
 * You may assume that the
 * message box ID has been
 * properly opened before this
 * call.
 * The message is 'nbytes' bytes
 * starting at offset 'msg'
 */
void do_mbox_send(mbox_t mbox, void *msg, int nbytes)
{
  /* TODO */
	int msg_num;
  	char buffer[MAX_MESSAGE_LENGTH];
	MessageBox *now_box=&MessageBoxen[mbox];

	semaphore_down(&now_box->full);
	lock_acquire(&now_box->lock);

	ASSERT(nbytes < MAX_MESSAGE_LENGTH);
	bcopy(msg,buffer,nbytes);
	msg_num = now_box->msg_num;
	(now_box->msgs[msg_num]).msg_len = nbytes;
	bcopy(buffer,(now_box->msgs[msg_num]).msg,nbytes);
	enqueue(&now_box->msg_queue,(node_t *)&now_box->msgs[msg_num]);
	now_box->msg_num++;

	lock_release(&now_box->lock);
	semaphore_up(&now_box->empty);
}


/* Receives a message from the
 * specified message box.  If
 * empty, the process will block
 * until it can remove an item.
 * You may assume that the
 * message box has been properly
 * opened before this call.
 * The message is copied into
 * 'msg'.  No more than
 * 'nbytes' bytes will by copied
 * into this buffer; longer
 * messages will be truncated.
 */
void do_mbox_recv(mbox_t mbox, void *msg, int nbytes)
{
  /* TODO */
	int msg_num;
	Message *buffer;
	MessageBox *now_box=&MessageBoxen[mbox];
	
	semaphore_down(&now_box->empty);
	lock_acquire(&now_box->lock);

	now_box->msg_num--;
	msg_num = now_box->msg_num;
	buffer = (Message *)dequeue(&now_box->msg_queue);
	ASSERT(nbytes <= buffer->msg_len);
	bcopy(buffer->msg,msg,nbytes);
	
	lock_release(&now_box->lock);
	semaphore_up(&now_box->full);	
}


