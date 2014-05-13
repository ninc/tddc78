!!!export OMP_NUM_THREADS=16
!!!ifort -O2 -xAVX -openmp -o intelopenMP laplsolvOpenMP.f90
!!!$ompsalloc ./laplsolve

program laplsolv
!-----------------------------------------------------------------------
! Serial program for solving the heat conduction problem 
! on a square using the Jacobi method. 
! Written by Fredrik Berntsson (frber@math.liu.se) March 2003
! Modified by Berkant Savas (besav@math.liu.se) April 2006
!-----------------------------------------------------------------------
  integer, parameter                  :: n=1000, maxiter=1000
  double precision,parameter          :: tol=1.0E-3
  double precision,dimension(0:n+1,0:n+1) :: T
  double precision,dimension(n)       :: left_bounds, right_bounds, leftb, rightb, tmp
  double precision                    :: error,x,local_error, tmp_error
  real                                :: t1,t0
  integer                             :: i,j,k, y_start, y_end, rank, threads, chunk_size, test
  character(len=20)                   :: str
  INTEGER, EXTERNAL :: OMP_GET_THREAD_NUM, OMP_GET_NUM_THREADS


  
  ! Set boundary conditions and initial values for the unknowns
  T=0.0D0
  T(0:n+1 , 0)     = 1.0D0
  T(0:n+1 , n+1)   = 1.0D0
  T(n+1   , 0:n+1) = 2.0D0
  

  ! Solve the linear system of equations using the Jacobi method
  call cpu_time(t0)

  do k=1,maxiter
     
     error=0.0D0
     


     ! =======================================================
     ! Setup threading segments
     ! =======================================================
 
     !$omp parallel shared(error, T)  private(local_error, y_start, y_end, rank, right_bounds, left_bounds, rightb, leftb, tmp, chunk_size, tmp_error, test, j, threads)
     threads = omp_get_num_threads()
     chunk_size = n/(threads) +1
     rank = omp_get_thread_num()
     y_start = rank*chunk_size + 1
     local_error = 0.0D0
     tmp_error = 0.0D0
     test = 0

    
     ! =======================================================
     ! Jacobi method
     ! =======================================================

     
     !$omp do schedule(STATIC)
     do j=1,n
        tmp = T(j, 1:n)
        test = test + 1
        ! Only do it the first time in the loop
        if(test == 1) then
           if(rank==threads-1) then
           
              !set value of LAST threads boundarys...
              
              right_bounds = T(n-1, 1:n)
              left_bounds = T(j, 1:n)
              !write(unit=*,fmt=*) 'rank: ', rank, 'left bound: ', j, 'right bounds: ', n-1
           else

              !set value of the other threads boundarys....

              right_bounds = T(j+chunk_size, 1:n)
              left_bounds = T(j-1, 1:n)
              !write(unit=*,fmt=*) 'rank: ', rank, 'left bound: ', j-1, 'right bounds: ', j+chunk_size
              
           end if
           
        end if

       ! Take correct values
       if(test == 1) then
           rightb = right_bounds
        else
           rightb = T(j-1, 1:n)
        end if
        
        if(test == chunk_size) then
           leftb = left_bounds
        else
           leftb = T(j+1, 1:n)
        end if; 

        ! Calculate temperature
        T(j, 1:n)=(T(j,0:n-1)+T(j, 2:n+1)+rightb+leftb)/4.0D0

        ! =======================================================
        ! Calculate the largest error in calculations
        ! =======================================================
        tmp_error = maxval(abs(tmp-T(j, 1:n)))
        
        if(tmp_error > local_error) then
           local_error = tmp_error
        end if


     end do
     !$omp end do

     !$omp barrier

     !$omp do schedule(static) reduction(MAX: error)
     do i=0,threads
        error = max(DBLE(error), DBLE(local_error))
     end do
     !$omp end do

     !$omp end parallel

     ! write(unit=*,fmt=*) 'Ending iteration: ', k

     if (error<tol) then 
        exit
     end if
  end do

  call cpu_time(t1)

  write(unit=*,fmt=*) 'Time:',t1-t0,'Number of Iterations:',k
  write(unit=*,fmt=*) 'Temperature of element T(1,1)  =',T(1,1)
  write(unit=*,fmt=*) 'Error estimate  =', error
  ! Uncomment the next part if you want to write the whole solution
  ! to a file. Useful for plotting. 
  
  open(unit=7,action='write',file='result.dat',status='unknown')
  write(unit=str,fmt='(a,i6,a)') '(',N,'F10.6)'
  do i=0,n+1
     write (unit=7,fmt=str) T(i,0:n+1)  
  end do
  close(unit=7)
  
end program laplsolv
