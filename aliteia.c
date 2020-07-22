/*
 * Get mem struct from pid
 * @Antonis: ekana allages koitakse tes thelei douleia akoma.
 *
 */

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmaengine.h>
#include <linux/freezer.h>
#include <linux/init.h>
//#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
//#include <linux/random.h>
#include <linux/slab.h>
#include <linux/wait.h>   
#include <linux/iommu.h>
#include <linux/pci.h>
#include <linux/device.h>
#include <linux/sched.h> //task_struct
#include <linux/pid.h> // pid_task, find_get_pid, pid_task
#include <linux/mm_types.h> // mm_struct, vm_area_struct
//#include <asm-generic/pgtable.h> //check if this include is valid
//#include <asm-generic/pgtable-nopud.h>
//#include <asm-generic/pgtable-nopmd.h>
//#include <../arch/arm64/include/asm/pgtable.h>

static unsigned int pid = 0;
module_param(pid, uint, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(pid, "Process ID (default: 0)");


/**
 * struct get_mem_struct_params - get_mem_struct parameters.
 * @pid:		Process ID
 */
struct get_mem_struct_params {
	unsigned int	pid;
};

/**
 * struct get_mem_struct_info - get_mem_struct information.
 * @params:		get_mem_struct parameters
 * @lock:		access protection to the fields of this structure
 */
static struct get_mem_struct_info {
	/* Test parameters */
	struct get_mem_struct_params	params;

	/* Internal state */
	struct mutex		lock;
	bool			did_init;
} get_mem_struct_info = {
	.lock = __MUTEX_INITIALIZER(get_mem_struct_info.lock),
};


static int get_mem_struct_run_set(const char *val, const struct kernel_param *kp);
static int get_mem_struct_run_get(char *val, const struct kernel_param *kp);
static const struct kernel_param_ops run_ops = {
	.set = get_mem_struct_run_set,
	.get = get_mem_struct_run_get,
};
static bool get_mem_struct_run;
module_param_cb(run, &run_ops, &get_mem_struct_run, S_IRUGO | S_IWUSR);
MODULE_PARM_DESC(run, "Run the get_mem_struct (default: false)");


/* poor man's completion - we want to use wait_event_freezable() on it */
struct get_mem_struct_done {
	bool			done;
	wait_queue_head_t	*wait;
};

static unsigned long  virt2phys(struct mm_struct* mm, unsigned long virt){
  unsigned long phys;
  struct page *page;
  pud_t *pud;
  pmd_t *pmd;
  pte_t *pte;
  pgd_t *pgd = pgd_offset(mm, virt);
  
  if (pgd_none(*pgd) || pgd_bad(*pgd))
    return 0;
  pud = pud_offset(pgd, virt);
  if (pud_none(*pud) || pud_bad(*pud))
    return 0;
  pmd = pmd_offset(pud, virt);
  if (pmd_none(*pmd) || pmd_bad(*pmd))
    return 0;
  if (!(pte = pte_offset_map(pmd, virt)))
    return 0;
  if (!(page = pte_page(*pte)))
    return 0;
  phys = page_to_phys(page);
  pte_unmap(pte);
  return phys;
}

static void run_get_mem_struct(struct get_mem_struct_info *info){
	
	struct get_mem_struct_params *params; 
	struct pid *pid_struct;
	struct task_struct *task;
	struct mm_struct *mm;
	struct vm_area_struct *vm_area, *tmp_vm_area;
    params = &info->params;
	printk("--------------------------------Begin of module--------------------------------\n");
	printk("\n\n\n");
	/* Copy run_get_mem parameters */
	params->pid = pid;
	printk("PID is: %u\n", params->pid);
	
	

	pid_struct = find_get_pid(params->pid);

	if(pid_struct!=NULL){

		task = pid_task(pid_struct, PIDTYPE_PID);

		mm = task->mm;
		vm_area = mm->mmap;
		
	    if(vm_area != NULL){
			printk("VM_START Address is: (dec)%lu  (hex)%lx\n", vm_area->vm_start, vm_area->vm_start);
	 		printk("VM_END Address is: (dec)%lu  (hex)%lx\n", vm_area->vm_end, vm_area->vm_end);
	 		//printk("VM Offset  is: %lu\n", vm_area->vm_pgoff); //Offset (within vm_file) in PAGE_SIZE units
			printk("\n\n\n");
			printk("Linked list of VM areas per task, sorted by address for PID %u is:\n", params->pid);
			printk("\n\n");
			tmp_vm_area = vm_area;
			while(tmp_vm_area != NULL){
	 			//dma_mmap_from_coherent(struct device *dev, vma_area, void *cpu_addr, size_t size, int *ret); //<- not sure that this call is present for us
	 			printk("VM_START Address is: (dec)%lu  (hex)%lx VM_END Address is: (dec)%lu  (hex)%lx \n", tmp_vm_area->vm_start, tmp_vm_area->vm_start, tmp_vm_area->vm_end, tmp_vm_area->vm_end);
                
	 			tmp_vm_area = tmp_vm_area->vm_next;
	 		}
		}
		printk("\n\n");
		printk("---------------------------------End of module---------------------------------\n");
		printk("\n\n\n");
        }
        else{

		printk("PID with value: %u does not exist!!!\n", params->pid);
		
        }

}


static void translate_struct(struct get_mem_struct_info *info)
{
  	struct get_mem_struct_params *params; 
	struct pid *pid_struct;
	struct task_struct *task;
	
	struct vm_area_struct *vma;
    unsigned long vpage;
    unsigned long phys;
	params = &info->params;
	printk("--------------------------------Begin of module--------------------------------\n");
	printk("\n\n\n");
	/* Copy run_get_mem parameters */
	params->pid = pid;
	printk("PID is: %u\n", params->pid);
    pid_struct = find_get_pid(params->pid);
    
    if(pid_struct){
      task = pid_task(pid_struct, PIDTYPE_PID);
      if (task->mm && task->mm->mmap){
        for (vma = task->mm->mmap; vma; vma = vma->vm_next){
          for (vpage = vma->vm_start; vpage < vma->vm_end; vpage += PAGE_SIZE){
            printf(KERN_INFO "Virtual page: 0x%lx\n", vpage);
            phys = virt2phys(task->mm, vpage);
            printk(KERN_INFO "Phys: 0x%lx\n",phys);
          }
        }
      }        
    }
    else{

		printk("PID with value: %u does not exist!!!\n", params->pid);
    }
    printk("---------------------------------End of module---------------------------------\n");
}


static void stop_get_mem_struct(void)
{
	//printk("Stopped the get_mem_struct.\n");
}

static void restart_get_mem_struct(struct get_mem_struct_info *info, bool run)
{
	
	if (!info->did_init)
		return;
	
	stop_get_mem_struct();
	
	run_get_mem_struct(info);
}



static int get_mem_struct_run_get(char *val, const struct kernel_param *kp)
{
	struct get_mem_struct_info *info = &get_mem_struct_info;

	mutex_lock(&info->lock);
	
	get_mem_struct_run = true;
	
	mutex_unlock(&info->lock);

	return param_get_bool(val, kp);
}

static int get_mem_struct_run_set(const char *val, const struct kernel_param *kp)
{
	struct get_mem_struct_info *info = &get_mem_struct_info;
	int ret;

	mutex_lock(&info->lock);
	ret = param_set_bool(val, kp);
	if (ret) {
		mutex_unlock(&info->lock);
		return ret;
	}
	if (get_mem_struct_run)
		restart_get_mem_struct(info, get_mem_struct_run);

	mutex_unlock(&info->lock);

	return ret;
}



static int __init get_mem_struct_init(void)
{


	struct get_mem_struct_info *info = &get_mem_struct_info;
	//struct get_mem_struct_params *params = &info->params;

	if (get_mem_struct_run) {
      //run_get_mem_struct(info);
        translate_struct(info);
	}
	info->did_init = true;


    return 0;
}
module_init(get_mem_struct_init);



static void __exit get_mem_struct_exit(void)
{
	//mutex_lock(&info->lock);
	printk("Goodbye get_mem_struct \n");
//	mutex_unlock(&info->lock);
 
}
module_exit(get_mem_struct_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FORTH-ICS");
MODULE_DESCRIPTION("Get mem struct from pid");
MODULE_VERSION("get_mem_struct");
