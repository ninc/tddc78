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
  integer, parameter                  :: n=1000, maxiter=1
  double precision,parameter          :: tol=1.0E-3
  double precision,dimension(0:n+1,0:n+1) :: T
  double precision,dimension(n)       :: upper_bounds, lower_bounds, upper, lower, tmp
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
     
     ! write(unit=*,fmt=*) 'Starting iteration: ', k

     error=0.0D0
     !Orginalet!
     !do j=1,n
     !   tmp2=T(1:n,j)
     !   T(1:n,j)=(T(0:n-1,j)+T(2:n+1,j)+T(1:n,j+1)+tmp1)/4.0D0
     !   error=max(error,maxval(abs(tmp2-T(1:n,j))))
     !   tmp1=tmp2
     !end do


     ! =======================================================
     ! Setup threading segments
     ! =======================================================
 
     ! write(unit=*,fmt=*) 'Starting Parallel section iteration: ', k

     !$omp parallel shared(error, T)  private(local_error, y_start, y_end, rank, upper_bounds, lower_bounds, upper, lower, tmp, threads, chunk_size, tmp_error, test, j)
     threads = omp_get_num_threads()
     chunk_size = n/(threads) +1
     rank = omp_get_thread_num()
     y_start = rank*chunk_size + 1
     local_error = 0.0D0
     tmp_error = 0.0D0
     test = 0

     ! Cut problem into segments for each thread
     if(rank == threads-1) then
        y_end = n-1;
        chunk_size = y_end - y_start
     else
        y_end = y_start + chunk_size
     end if

     ! Save boundaries
     upper_bounds = T(1:n,y_start-1)
     lower_bounds = T(1:n,y_end + 1)



     ! =======================================================
     ! Jacobi method
     ! =======================================================

      ! write(unit=*,fmt=*) 'Starting Jacobi iteration: ', k

     !$omp do schedule(STATIC)
     do j=1,n
        tmp = T(1:n, j)
        test = test + 1
        ! Only do it the first time in the loop
        if(test == 1) then
           upper_bounds = T(1:n, j-1)
           lower_bounds = T(1:n, j+chunk_size)
           ! write(unit=*,fmt=*) 'rank: ', rank, 'left bound: ', j-1, 'right bounds: ', j+chunk_size
           write(unit=*,fmt=*) 'rank: ', rank, 'j: ', j, 'n', n
        end if

       ! Take correct values
       if(test == 1) then
           upper = upper_bounds
        else
           upper = T(1:n, j-1)
        end if
        
        if(test == chunk_size) then
           lower = lower_bounds
        else
           lower = T(1:n, j+1)
        end if; 

        ! Calculate temperature
        T(1:n,j)=(T(0:n-1,j)+T(2:n+1,j)+upper+lower)/4.0D0

        ! =======================================================
        ! Calculate the largest error in calculations
        ! =======================================================
        tmp_error = maxval(abs(tmp-T(1:n,j)))
        
        if(tmp_error > local_error) then
           local_error = tmp_error
        end if


     end do
     ! write(unit=*,fmt=*) 'j=: ', j, 'test=', test, 'rank=', rank

     !do j=y_start,y_end

        ! Save old values for error, comparison
        !tmp = T(1:n, j)
        ! Calculate boundary conditions
        !if(j == y_start) then
        !   upper = upper_bounds
        !else
        !   upper = T(1:n, j-1)
        !end if
        
        !if(j == y_end) then
        !   lower = lower_bounds
        !else
        !   lower = T(1:n, j+1)
        !end if; 

        !T(1:n,j)=(T(0:n-1,j)+T(2:n+1,j)+lower+upper)/4.0D0
        
        ! =======================================================
        ! Calculate the largest error in calculations
        ! =======================================================
        !tmp_error = maxval(abs(tmp-T(1:n,j)))
        
        !if(tmp_error > local_error) then
        !   local_error = tmp_error
        !end if
     !end do

     !$omp barrier

     !!!$omp do schedule(static) reduction(MAX: error)
     !!do i=0,threads
     !!   error = max(DBLE(error), DBLE(local_error))
     !!end do
     !!$omp end do

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