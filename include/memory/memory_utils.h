#pragma once
 
#if !defined(SAFE_DELETE)
/// <summary>
/// Delete a pointer and set it to nullptr, but only if it's not already null.
/// </summary>
#define SAFE_DELETE(ptr) if(ptr) delete ptr; ptr=nullptr;
#endif

#if !defined(SAFE_DELETE_ARRAY)
/// <summary>
/// Delete an array and set it to nullptr, but only if it's not already null.
/// </summary>
#define SAFE_DELETE_ARRAY(arr) if (arr) delete [] arr; arr=nullptr; 
#endif
