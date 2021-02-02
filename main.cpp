#include   <iostream>
#include   <vector>
#include   <malloc.h>
#include   <chrono>
#include   <unistd.h>
#include    <thread>
#include    <mutex>
#include    <string.h>
#include    <atomic>
#include    <functional>


namespace  haisql
{
typedef   unsigned long long   timet_nanoseconds;
inline  volatile timet_nanoseconds   now_steady_nanoseconds( void ) noexcept
{
    return  std::chrono::duration_cast<std::chrono::nanoseconds>( std::chrono::steady_clock::now().time_since_epoch() ).count();
}
}//  namespace  haisql


//  这里用于测试vt容器自身的消耗评估，与下面test_malloc()类似的开销,  这种vt开销作为一个参考，malloc大约0.5ns, malloc+free大约 1ns,
//  实际并没有扣除这个消耗，只是测试出来，作为一个参考
void   test_vt_use_nanosecond( std::vector<void*> &vt_ptr_void_in, const unsigned int uint_test_count_in, const bool bool_show_in )
{
    const haisql::timet_nanoseconds  timet_nanoseconds_begin = haisql::now_steady_nanoseconds();
    for( size_t i=0; i<uint_test_count_in; ++i )
    {
        vt_ptr_void_in[i] = reinterpret_cast<void*>( i );//write指针
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_malloc_end = haisql::now_steady_nanoseconds();
    // size_sum 用于避免编译器优化掉无实际作用循环 read 代码
    size_t   size_sum  = 0;
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        size_sum += reinterpret_cast<size_t>( vt_ptr_void_in[i] );//read指针
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_free_end = haisql::now_steady_nanoseconds();
    const unsigned long long  ulong_malloc_use_nansecond = timet_nanoseconds_malloc_end - timet_nanoseconds_begin;
    const unsigned long long  ulong_malloc_free_use_nansecond = timet_nanoseconds_free_end - timet_nanoseconds_begin;
    if( bool_show_in )
    {
        std::cout << std::endl;
        std::cout << "test_vt_use_nanosecond vt write() use="  << ulong_malloc_use_nansecond << ", uint_test_count_in=" << uint_test_count_in << ", avg=" << 1.0*( ulong_malloc_use_nansecond ) / uint_test_count_in << std::endl;
        std::cout << "test_vt_use_nanosecond vt write+read() use=" << ulong_malloc_free_use_nansecond << ", uint_test_count_in=" << uint_test_count_in << ", avg=" << 1.0*(ulong_malloc_free_use_nansecond)/uint_test_count_in << std::endl;
        std::cout << "test_vt_use_nanosecond size_sum=" << size_sum << std::endl;   // 这行不能删，用于避免编译器优化掉无实际作用代码
    }
    return;
}


void   test_malloc( const unsigned int uint_malloc_size_in, const unsigned int uint_test_count_in, const bool bool_show_in )
{
    std::vector<void*>  vt_ptr_void;
    vt_ptr_void.resize( uint_test_count_in, nullptr );
    const haisql::timet_nanoseconds  timet_nanoseconds_begin = haisql::now_steady_nanoseconds();
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        vt_ptr_void[i] = malloc( uint_malloc_size_in );
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_malloc_end = haisql::now_steady_nanoseconds();
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        free( vt_ptr_void[i] );
        continue;
    }
    if( bool_show_in )
    {
        const haisql::timet_nanoseconds  timet_nanoseconds_free_end = haisql::now_steady_nanoseconds();
        const unsigned long long  ulong_malloc_use_nansecond = timet_nanoseconds_malloc_end - timet_nanoseconds_begin;
        const unsigned long long  ulong_malloc_free_use_nansecond = timet_nanoseconds_free_end - timet_nanoseconds_begin;
        char   chars_malloc_size[32];
        if( uint_malloc_size_in >= 4096 && !( uint_malloc_size_in & 4095 ) )
        {
            sprintf( chars_malloc_size, "4KB*%u", uint_malloc_size_in/4096 );
        }
        else
        {
            sprintf( chars_malloc_size, "%u", uint_malloc_size_in );
        }
        std::cout << "malloc()  " << chars_malloc_size <<"   use="  << ulong_malloc_use_nansecond << ", avg=" << ( ulong_malloc_use_nansecond ) / uint_test_count_in
                  <<  std::endl;
        std::cout << "malloc+free() " << chars_malloc_size << " use=" << ulong_malloc_free_use_nansecond << ", avg=" << ulong_malloc_free_use_nansecond/uint_test_count_in
                  << std::endl;
    }
    return;
}


//   uint_malloc_size_in  应该是 2 的N次方
void   test_malloc_range( const unsigned int uint_malloc_size_in, const unsigned int uint_test_count_in, const bool bool_show_in )
{
    std::vector<void*>  vt_ptr_void;
    vt_ptr_void.resize( uint_test_count_in, nullptr );

    const haisql::timet_nanoseconds  timet_nanoseconds_begin = haisql::now_steady_nanoseconds();
    const unsigned int  uint_malloc_size_mask = uint_malloc_size_in - 1;
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        vt_ptr_void[i] = malloc( ( i & uint_malloc_size_mask ) + 1 );
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_malloc_end = haisql::now_steady_nanoseconds();
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        free( vt_ptr_void[i] );
        continue;
    }
    if( bool_show_in )
    {
        const haisql::timet_nanoseconds  timet_nanoseconds_free_end = haisql::now_steady_nanoseconds();
        const unsigned long long  ulong_malloc_use_nansecond = timet_nanoseconds_malloc_end - timet_nanoseconds_begin;
        const unsigned long long  ulong_malloc_free_use_nansecond = timet_nanoseconds_free_end - timet_nanoseconds_begin;
        std::cout << "malloc() 1-" << uint_malloc_size_in << "Byte use="  << ulong_malloc_use_nansecond << ", avg=" << ulong_malloc_use_nansecond / uint_test_count_in
                  <<  std::endl;
        std::cout << "malloc+free() 1-" << uint_malloc_size_in << "Byte use=" << ulong_malloc_free_use_nansecond << ", avg=" << ulong_malloc_free_use_nansecond /uint_test_count_in
                  << std::endl;
    }
    return;
}


void   test_malloc_real_random( const unsigned int uint_malloc_size_in, const unsigned int uint_test_count_in, const bool bool_show_in )
{
    std::vector<unsigned int>  vt_uint_random;
    vt_uint_random.reserve( uint_test_count_in );
    const unsigned int  uint_malloc_size_mask = uint_malloc_size_in - 1;
    srand( 12345678 );
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        const unsigned int  uint_rand = ( rand() & uint_malloc_size_mask ) + 1;
        vt_uint_random.push_back( uint_rand );
        continue;
    }
    std::vector<void*>  vt_ptr_void;
    vt_ptr_void.resize( uint_test_count_in, nullptr );

    const haisql::timet_nanoseconds  timet_nanoseconds_begin = haisql::now_steady_nanoseconds();
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        vt_ptr_void[i] = malloc( vt_uint_random[i] );
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_malloc_end = haisql::now_steady_nanoseconds();
    for( unsigned int i=0; i<uint_test_count_in; ++i )
    {
        free( vt_ptr_void[i] );
        continue;
    }
    if( bool_show_in )
    {
        const haisql::timet_nanoseconds  timet_nanoseconds_free_end = haisql::now_steady_nanoseconds();
        const unsigned long long  ulong_malloc_use_nansecond = timet_nanoseconds_malloc_end - timet_nanoseconds_begin;
        const unsigned long long  ulong_malloc_free_use_nansecond = timet_nanoseconds_free_end - timet_nanoseconds_begin;
        std::cout << "malloc() 1-" << uint_malloc_size_in << "Byte real random use="  << ulong_malloc_use_nansecond << ", avg=" << ulong_malloc_use_nansecond / uint_test_count_in
                  << std::endl;
        std::cout << "malloc+free() 1-" << uint_malloc_size_in << "Byte real random use=" << ulong_malloc_free_use_nansecond << ", avg=" << ulong_malloc_free_use_nansecond /uint_test_count_in
                  << std::endl;
    }
    return;
}


void  test_run( const unsigned int  uint_test_count_in, const bool bool_show_in ) noexcept
{
    //  测试 1-256字节
    test_malloc_range( 256, uint_test_count_in, bool_show_in );
    test_malloc_real_random( 256, uint_test_count_in, bool_show_in );
    //  测试 1-4096字节
    test_malloc_range( 4096, uint_test_count_in, bool_show_in );
    test_malloc_real_random( 4096, uint_test_count_in, bool_show_in );
    test_malloc( 8, uint_test_count_in, bool_show_in );
    for( unsigned int j=1; j<=10; ++j )
    {
        test_malloc( j*16, uint_test_count_in, bool_show_in );
        continue;
    }
    {
        const unsigned int  uint_test128_count = uint_test_count_in / 10;
        for( unsigned int j=2; j<16; ++j )
        {
            test_malloc( j*128, uint_test128_count, bool_show_in );
            continue;
        }
        for( unsigned int j=4; j<8; ++j )
        {
            test_malloc( j*512, uint_test128_count, bool_show_in );
            continue;
        }
    }
    {
        const unsigned int  uint_test1k_count = uint_test_count_in / 10;
        for( unsigned int j=1; j<=10; ++j )
        {
            test_malloc( j*1024, uint_test1k_count, bool_show_in );
            continue;
        }
    }
    {
        const unsigned int  uint_test4k_count = uint_test_count_in / 100;
        for( unsigned int j=1; j<=8; ++j )
        {
            test_malloc( j*4096, uint_test4k_count, bool_show_in );
            continue;
        }
    }
    {
        const unsigned int  uint_test64k_count = uint_test_count_in / 1600;
        for( unsigned int j=1; j<=8; ++j )
        {
            test_malloc( j*16*4096, uint_test64k_count, bool_show_in );
            continue;
        }
    }
    return;
}


std::atomic<unsigned int>   g_atomic_uint_thread_join_count( 0 );


void  test_loop( const unsigned int  uint_test_count_in, const unsigned int uint_loop_count_in, const bool bool_show_in, const unsigned int uint_thread_number_in ) noexcept
{
    free( malloc( 1 ) );
    auto   auto_begin = haisql::now_steady_nanoseconds();
    //  下面测试最大使用2G多的内存
    for( unsigned int i=0; i<uint_loop_count_in; ++i )
    {
        if( bool_show_in )
        {
            std::cout << std::endl << "memory loop i=" << i << std::endl;
        }
        auto   auto_begin_in_loop = haisql::now_steady_nanoseconds();
        test_run( uint_test_count_in, bool_show_in );
        auto   auto_end_in_loop = haisql::now_steady_nanoseconds();
        if( bool_show_in )
        {
            std::cout << "test_loop() in loop " << i << " use nanoseconds=" << auto_end_in_loop - auto_begin_in_loop << std::endl;
        }
        continue;
    }
    const unsigned long long  use_nanoseconds = haisql::now_steady_nanoseconds() - auto_begin;
    const unsigned int uint_thread_join_count = g_atomic_uint_thread_join_count.fetch_add( 1, std::memory_order_relaxed );
    char  chars_out[1024];
    sprintf( chars_out, "Log: test_loop() loop_count=%u, use nanoseconds=%llu, thread_number=%u, thread_join_count=%u\r\n",
             uint_loop_count_in, use_nanoseconds, uint_thread_number_in, uint_thread_join_count );
    fputs( chars_out, stderr );
    return;
}


void   thread_bind_1cpu( const unsigned int uint_thread_number_in )
{
    // 绑定到1个CPU上，减少线程的无效切换
    {
        const unsigned int  uint_cpu_size = std::thread::hardware_concurrency();
        const unsigned int  uint_bind_cpu1 = uint_thread_number_in % uint_cpu_size;
        cpu_set_t   cpu_set_new;
        CPU_ZERO( &cpu_set_new );
        CPU_SET( uint_bind_cpu1, &cpu_set_new );
        if( pthread_setaffinity_np( pthread_self(), sizeof( cpu_set_new ), &cpu_set_new ) )
        {
            std::cerr << "Error: thread_bind_1cpu() pthread_setaffinity_np() failed. bind_cpu1=" << uint_bind_cpu1 << std::endl;
        }
    }
    return;
}


std::vector<void*>   g_ar_vt_ptr_void[128];
std::atomic<unsigned int>   g_atomic_uint_free_count( 0 );
//  用于测试跨线程释放
std::mutex    g_ar_mutex[128];


void  test_loop_thread( const unsigned int uint_test_count_in, const unsigned int uint_loop_count_in, const bool bool_show_in, const unsigned int uint_thread_number_in ) noexcept
{
    thread_bind_1cpu( uint_thread_number_in );
    test_loop( uint_test_count_in, uint_loop_count_in, bool_show_in, uint_thread_number_in );
    return;
}


void  test_many_thread_cross_free( const unsigned int uint_thread_number_in, const unsigned int uint_max_size_in ) noexcept
{
    std::mutex   &mutex1 =  g_ar_mutex[uint_thread_number_in];
    std::vector<void*>  &vt_tmp1 = g_ar_vt_ptr_void[uint_thread_number_in];
    unsigned int  uint_count_sum = 0;
    while( 1 )
    {
        if( vt_tmp1.size() )
        {
            std::lock_guard<std::mutex>   lock1( mutex1 );
            if( vt_tmp1.size() )  //  double check
            {
                const unsigned int uint_count = vt_tmp1.size();
                uint_count_sum += uint_count;
                for( unsigned int i=0; i<uint_count; ++i )
                {
                    free( vt_tmp1[i] );
                    continue;
                }
                vt_tmp1.clear();
                g_atomic_uint_free_count.fetch_add( uint_count );
            }
        }
        if( g_atomic_uint_free_count.load( std::memory_order_relaxed ) == uint_max_size_in )
        {
            break;
        }
        usleep( 40 );
        continue;
    }
    vt_tmp1.shrink_to_fit();
    const unsigned int uint_thread_join_count = g_atomic_uint_thread_join_count.fetch_add( 1, std::memory_order_relaxed );
    char  chars_out[1024];
    sprintf( chars_out, "Log: test_many_thread_cross_free() thread_number=%u, count=%u, thread_join_count=%u\r\n",
             uint_thread_number_in, uint_count_sum, uint_thread_join_count );
    fputs( chars_out, stderr );
    return;
}


void  test_main_thread_cross_malloc( const unsigned int uint_max_size_in, const unsigned int uint_thread_count_in ) noexcept
{
    for( unsigned int i=0; i<uint_thread_count_in; ++i )
    {
        g_ar_vt_ptr_void[i].reserve( 400 );
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_begin = haisql::now_steady_nanoseconds();
    unsigned int  uint_malloc_count = 0;
    while( 1 )
    {
        for( unsigned int i=0; i<uint_thread_count_in; ++i )
        {
            std::mutex   &mutex1 =  g_ar_mutex[i];
            std::vector<void*>  &vt_tmp1 = g_ar_vt_ptr_void[i];
            if( vt_tmp1.empty() )
            {
                std::lock_guard<std::mutex>   lock1( mutex1 );
                if( vt_tmp1.empty() )  //double check
                {
                    for( unsigned int j=0; j<400; ++j )
                    {
                        vt_tmp1.push_back( malloc( 8 ) );
                        continue;
                    }
                    uint_malloc_count += 400;
                }
            }
            if( uint_malloc_count >= uint_max_size_in )
            {
                break;
            }
            continue;
        }
        if( uint_malloc_count >= uint_max_size_in )
        {
            break;
        }
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_use = haisql::now_steady_nanoseconds() - timet_nanoseconds_begin;
    const unsigned int  uint_avg = timet_nanoseconds_use / uint_malloc_count;
    std::cout << std::endl;
    std::cout << "Log: test_main_thread_cross_malloc() malloc_count=" << uint_malloc_count << ", thread_count=" << uint_thread_count_in << ", use_nanoseconds="<< timet_nanoseconds_use << ", avg=" << uint_avg << std::endl;
    return;
}


//  用于测试跨线程 NO SYNC 释放
std::array<volatile size_t,400>   g_ar_ar_sizet[128];


void  test_many_thread_cross_free_no_sync( const unsigned int uint_thread_number_in, const unsigned int uint_max_size_in ) noexcept
{
    std::array<volatile size_t,400>   &ar_sizet = g_ar_ar_sizet[uint_thread_number_in];
    unsigned int  uint_count_sum = 0;
    while( 1 )
    {
        if( ar_sizet[0] )
        {
            for( unsigned int i=0; i<400; )
            {
                void*  ptr_void_tmp = reinterpret_cast<void*>( ar_sizet[i] );
                if( ptr_void_tmp )
                {
                    free( ptr_void_tmp );
                    ++i;
                }
                continue;
            }
            for( unsigned int i=0; i<400; ++i )
            {
                ar_sizet[i] = 0;
                continue;
            }
            uint_count_sum += 400;
            g_atomic_uint_free_count.fetch_add( 400,  std::memory_order_relaxed );
        }
        if( g_atomic_uint_free_count.load( std::memory_order_relaxed ) >= uint_max_size_in )
        {
            break;
        }
        usleep( 10 );
        continue;
    }
    const unsigned int uint_thread_join_count = g_atomic_uint_thread_join_count.fetch_add( 1, std::memory_order_relaxed );
    char  chars_out[1024];
    sprintf( chars_out, "Log: test_many_thread_cross_free_no_sync() thread_number=%u, count=%u, thread_join_count=%u\r\n",
             uint_thread_number_in, uint_count_sum, uint_thread_join_count );
    fputs( chars_out, stderr );
    return;
}


void  test_main_thread_cross_malloc_no_sync( const unsigned int uint_max_size_in, const unsigned int uint_thread_count_in ) noexcept
{
    for( unsigned int i=0; i<uint_thread_count_in; ++i )
    {
        std::array<volatile size_t,400>   &ar_sizet = g_ar_ar_sizet[i];
        for( unsigned int j=0; j<400; ++j )
        {
            ar_sizet[j] = 0;
            continue;
        }
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_begin = haisql::now_steady_nanoseconds();
    unsigned int  uint_malloc_count = 0;
    while( 1 )
    {
        for( unsigned int i=0; i<uint_thread_count_in; ++i )
        {
            std::array<volatile size_t,400>   &ar_sizet = g_ar_ar_sizet[i];
            if( !ar_sizet[0] )
            {
                for( unsigned int j=0; j<400;  )
                {
                    if( !ar_sizet[j] )
                    {
                        ar_sizet[j] = reinterpret_cast<size_t>( malloc( 8 ) );
                        ++j;
                    }
                    continue;
                }
                uint_malloc_count += 400;
            }
            if( uint_malloc_count >= uint_max_size_in )
            {
                break;
            }
            continue;
        }
        if( uint_malloc_count >= uint_max_size_in )
        {
            break;
        }
        continue;
    }
    const haisql::timet_nanoseconds  timet_nanoseconds_use = haisql::now_steady_nanoseconds() - timet_nanoseconds_begin;
    const unsigned int  uint_avg = timet_nanoseconds_use / uint_malloc_count;
    std::cout << std::endl;
    std::cout << "Log: test_main_thread_cross_malloc() malloc_count=" << uint_malloc_count << ", thread_count=" << uint_thread_count_in << ", use_nanoseconds="<< timet_nanoseconds_use << ", avg=" << uint_avg << std::endl;
    return;
}


void   check_malloc_usable_size(  const unsigned int uint_malloc_size_in )  noexcept
{
    void*  ptr_void_tmp = malloc( uint_malloc_size_in );
    const unsigned int  uint_malloc_usable_size = malloc_usable_size( ptr_void_tmp );
    const unsigned int  uint_waste_size = uint_malloc_usable_size - uint_malloc_size_in;
    const unsigned int  uint_waste_rate = uint_waste_size* 100 / uint_malloc_usable_size;
    std::cout << "Log: check_malloc_usable_size() malloc() malloc_size=" << uint_malloc_size_in << ", malloc_usable_size=" << uint_malloc_usable_size;
    if( uint_waste_size )
    {
        std::cout << ", waste_size=" << uint_waste_size << ", waste_rate=" << uint_waste_rate << "%";
    }
    std::cout << std::endl;
    free( ptr_void_tmp );
    return;
}


//  下面这个是一种检测，测试内存分配库内部碎片的情况,  但是实际情况要复杂的多，这里没有评估外部碎片的情况，Linux ptmalloc库有巨大的外部碎片。
void   check_malloc_usable_size_run( void )  noexcept
{
    for( unsigned int i=1; i<4; ++i )
    {
        check_malloc_usable_size( i*8 );
        continue;
    }
    for( unsigned int i=1; i<(64*3); ++i )
    {
        check_malloc_usable_size( i*16 );
        continue;
    }
    for( unsigned int i=1; i<=10; ++i )
    {
        check_malloc_usable_size( i*1024 );
        continue;
    }
    return;
}


int main( int argc,char *argv[] )
{
    if(  argc > 1 && 0 == strcmp( argv[1], "--help" ) )
    {
        std::cout << "Usage: test_use_so  test_count  loop_count  thread_count" << std::endl;
        std::cout << "Default:  test_count 1000000"  << std::endl;
        std::cout << "Default:  loop_count 2"  << std::endl;
        std::cout << "Default:  thread_count 1"  << std::endl;
        return  0;
    }

    unsigned int uint_test_count = 1000000;
    if( argc > 1 )
    {
        unsigned int uint_tmp = atoi( argv[1] );
        if( uint_tmp )
        {
            //  不允许低于1万次
            if( uint_tmp < 10000 )
            {
                uint_tmp = 10000;
            }
            uint_test_count = uint_tmp;
        }
    }
    unsigned int uint_loop_count = 2;
    if( argc > 2 )
    {
        const unsigned int uint_tmp = atoi( argv[2] );
        if( uint_tmp )
        {
            uint_loop_count = uint_tmp;
        }
    }
    unsigned int uint_thread_count = 1;
    if( argc > 3 )
    {
        unsigned int uint_tmp = atoi( argv[3] );
        if( uint_tmp > 1 )
        {
            const unsigned int uint_max_thread = std::thread::hardware_concurrency() * 100;
            //  限制每个CPU最多100个线程
            if( uint_tmp > uint_max_thread )
            {
                uint_tmp = uint_max_thread;
            }
            uint_thread_count = uint_tmp;
        }
    }

    {
        char*  ptr_char_tmp = reinterpret_cast<char*>( malloc( 128 ) );
        sprintf( ptr_char_tmp,  "Log: enter main(). test_count=%u, loop_count=%u, thread_count=%u\r\n", uint_test_count, uint_loop_count, uint_thread_count );
        fputs( ptr_char_tmp, stdout );
        free( ptr_char_tmp );
    }

    check_malloc_usable_size_run();
    malloc_stats();

    //  测试代码本身vt内部data write/read 的开销．这部分开销大约1ns,  这里只是测试，并没有在后面扣除掉这部分开销，所以真实的性能还要高很多
    {
        std::vector<void*>  vt_ptr_void;
        vt_ptr_void.resize( uint_test_count, nullptr );
        test_vt_use_nanosecond( vt_ptr_void, uint_test_count, true );
    }

    //  下面这部分代码测试了多个线程工作的场景
    if( uint_thread_count > 1 )
    {
        std::vector<std::unique_ptr<std::thread> >    vt_up_thread;
        vt_up_thread.reserve( uint_thread_count );
        for( unsigned int i=1; i<uint_thread_count; ++i )
        {
            vt_up_thread.emplace_back( std::make_unique<std::thread>( std::bind( test_loop_thread, uint_test_count, uint_loop_count, false, i ) ) );
            continue;
        }
        std::cout << "Log: main thread begin test multi_thread run()\r\n" << std::endl;
        //  主线程执行，　允许显示
        test_loop( uint_test_count, uint_loop_count, true, 0 );
        std::cout << "Log: main thread end test multi_thread run(), wait thread join.\r\n" << std::endl;
        for( unsigned int i=0, n=vt_up_thread.size(); i<n; ++i )
        {
            vt_up_thread[i]->join();
            continue;
        }
    }
    else
    {
        //  单一主线程执行，　允许显示
        test_loop( uint_test_count, uint_loop_count, true, 0 );
    }

    //  下面这部分代码测试了主线程作为生产者malloc，其他线程作为消费者提交free的场景
    if( uint_thread_count > 1 )
    {
        if( uint_thread_count > 100 )
        {
            uint_thread_count = 100;
        }
        //  至少测试1000万次
        if( uint_test_count < 10000000 )
        {
            uint_test_count = 10000000;
        }
        g_atomic_uint_free_count.store( 0, std::memory_order_relaxed );
        g_atomic_uint_thread_join_count.store( 0, std::memory_order_relaxed );
        std::cout << std::endl;
        std::cout << "Log: begin test one producer, muti_consumer="  << uint_thread_count << ", test_count=" << uint_test_count << std::endl;
        std::vector<std::unique_ptr<std::thread> >    vt_up_thread;
        vt_up_thread.reserve( uint_thread_count );
        for( unsigned int i=0; i<uint_thread_count; ++i )
        {
            vt_up_thread.emplace_back( std::make_unique<std::thread>( std::bind( test_many_thread_cross_free, i, uint_test_count ) ) );
            continue;
        }
        std::cout << "Log: begin test one thread producer multi_thread consumer()\r\n" << std::endl;
        //  主线程执行作为生产者，生产10M　
        test_main_thread_cross_malloc( uint_test_count, uint_thread_count );
        std::cout << "Log: end test one thread producer multi_thread consumer(), wait thread join\r\n" << std::endl;
        for( unsigned int i=0, n=vt_up_thread.size(); i<n; ++i )
        {
            vt_up_thread[i]->join();
            continue;
        }
        std::cout << "Log: end test one thread producer multi_thread consumer(), end thread join\r\n" << std::endl;
    }


    if( uint_thread_count > 1 )
    {
        /*if( uint_thread_count > 8 )
        {
            uint_thread_count = 8;
        }*/
        g_atomic_uint_free_count.store( 0, std::memory_order_relaxed );
        g_atomic_uint_thread_join_count.store( 0, std::memory_order_relaxed );
        std::cout << std::endl;
        std::cout << "Log: begin test NO SYNC one producer, muti_consumer="  << uint_thread_count << ", test_count=" << uint_test_count << std::endl;
        std::vector<std::unique_ptr<std::thread> >    vt_up_thread;
        vt_up_thread.reserve( uint_thread_count );
        for( unsigned int i=0; i<uint_thread_count; ++i )
        {
            vt_up_thread.emplace_back( std::make_unique<std::thread>( std::bind( test_many_thread_cross_free_no_sync, i, uint_test_count ) ) );
            continue;
        }
        std::cout << "Log: begin test NO SYNC one thread producer multi_thread consumer()\r\n" << std::endl;
        //  主线程执行作为生产者，生产10M　
        test_main_thread_cross_malloc_no_sync( uint_test_count, uint_thread_count );
        std::cout << "Log: end test NO SYNC one thread producer multi_thread consumer(), wait thread join\r\n" << std::endl;
        for( unsigned int i=0, n=vt_up_thread.size(); i<n; ++i )
        {
            vt_up_thread[i]->join();
            continue;
        }
        std::cout << "Log: end test NO SYNC one thread producer multi_thread consumer(), end thread join\r\n" << std::endl;
        vt_up_thread.clear();
    }

    malloc_stats();
    usleep( 2000000 );
    malloc_stats();
    usleep( 2000000 );
    malloc_stats();
    usleep( 2000000 );
    malloc_stats();
    usleep( 2000000 );
    malloc_stats();
    usleep( 2000000 );
    malloc_stats();
    return 0;
}
