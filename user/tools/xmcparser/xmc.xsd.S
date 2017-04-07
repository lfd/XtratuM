/*<?xml version="1.0"?>*/
<xs:schema targetNamespace="http://www.xtratum.org/xm-2.3" 
	   xmlns:xs="http://www.w3.org/2001/XMLSchema"
	   xmlns="http://www.xtratum.org/xm-2.3"
	   elementFormDefault="qualified" 
	   attributeFormDefault="unqualified">
#if defined(ia32)
  <xs:include schemaLocation="ia32/xmc_ia32.xsd"/>
#else
#error No architecture defined
#endif
	
#ifdef CONFIG_DEV_PC_VGA
  <xs:include schemaLocation="devices/xmc_pc_vga.xsd"/>
#endif
	
#ifdef CONFIG_DEV_PC_UART
  <xs:include schemaLocation="devices/xmc_pc_uart.xsd"/>
#endif
	
#ifdef CONFIG_DEV_MEMBLOCK
 <xs:include schemaLocation="devices/xmc_memblock.xsd"/>
#endif
  <!-- Basic types definition -->
  <xs:simpleType name="id_t">
    <xs:restriction base="xs:integer">
      <xs:minInclusive value="0"/>
    </xs:restriction>
  </xs:simpleType>
  
  <xs:simpleType name="partitionId_t">
    <xs:restriction base="xs:integer">
      <xs:minInclusive value="0"/>
    </xs:restriction>
  </xs:simpleType>

  <xs:simpleType name="idString_t">
    <xs:restriction base="xs:string">
      <xs:minLength value="1"/>
      <xs:maxLength value="# CONFIG_ID_STRING_LENGTH #"/>
    </xs:restriction>
  </xs:simpleType>

  <xs:simpleType name="processorId_t">
    <xs:restriction base="xs:integer">
      <xs:minInclusive value="0"/>
      <xs:maxExclusive value="256"/>
    </xs:restriction>
  </xs:simpleType>

  <xs:simpleType name="hwIrqId_t">
    <xs:restriction base="xs:integer">
      <xs:minInclusive value="0"/>
      <xs:maxExclusive value="# CONFIG_NO_HWIRQS #"/>
    </xs:restriction>
  </xs:simpleType>
	
  <xs:simpleType name="hwIrqLine_t">
    <xs:list itemType="hwIrqId_t"/>
  </xs:simpleType>
	
  <xs:simpleType name="hex_t">
    <xs:restriction base="xs:string">
      <xs:pattern value="0x[0-9a-fA-F]+"/>
    </xs:restriction>
  </xs:simpleType>
  <xs:simpleType name="version_t">
    <xs:restriction base="xs:string">
      <xs:pattern value="[0-9]+.[0-9]+.[0-9]+"/>
    </xs:restriction>
  </xs:simpleType>

  <xs:simpleType name="freqUnit_t">
    <xs:restriction base="xs:string">
      <xs:pattern value="[0-9]+(.[0-9]+)?([MK][Hh]z)"/>
    </xs:restriction>
  </xs:simpleType>
	
  <xs:simpleType name="processorFeatures_t">
    <xs:list itemType="cpuFeatList_t"/>
  </xs:simpleType>

  <xs:simpleType name="partitionFlagsEnum_t">
    <xs:restriction base="xs:string">
      <xs:enumeration value="sv"/>
      <xs:enumeration value="boot"/>
#ifdef CONFIG_SPARE_SCHEDULING
      <xs:enumeration value="sphost"/>
#endif
    </xs:restriction>
  </xs:simpleType>
	
  <xs:simpleType name="partitionFlags_t">
    <xs:list itemType="partitionFlagsEnum_t"/>
  </xs:simpleType>
	
  <xs:simpleType name="memAreaFlagsEnum_t">
    <xs:restriction base="xs:string">
      <xs:enumeration value="shared"/>
      <xs:enumeration value="mapped"/>
      <xs:enumeration value="read"/>
      <xs:enumeration value="write"/>
      <xs:enumeration value="exec"/>
      <xs:enumeration value="rom"/>
      <xs:enumeration value="flag0"/>
      <xs:enumeration value="flag1"/>
      <xs:enumeration value="flag2"/>
      <xs:enumeration value="flag3"/>
    </xs:restriction>
  </xs:simpleType>
	
  <xs:simpleType name="memAreaFlags_t">
    <xs:list itemType="memAreaFlagsEnum_t"/>
  </xs:simpleType>
	
  <xs:simpleType name="sizeUnit_t">
    <xs:restriction base="xs:string">
      <xs:pattern value="[0-9]+(.[0-9]+)?([MK]?B)"/>
    </xs:restriction>
  </xs:simpleType>

  <xs:simpleType name="timeUnit_t">
    <xs:restriction base="xs:string">
      <xs:pattern value="[0-9]+(.[0-9]+)?([mu]?[sS])"/>
    </xs:restriction>
  </xs:simpleType>
  <xs:simpleType name="hmString_t">
    <xs:restriction base="xs:string">
      <!-- <track id="xml-list-hm-events" > -->

      <xs:enumeration value="XM_HM_EV_INTERNAL_ERROR"/>
      <xs:enumeration value="XM_HM_EV_UNEXPECTED_TRAP"/>
      <xs:enumeration value="XM_HM_EV_PARTITION_ERROR"/>
      <xs:enumeration value="XM_HM_EV_PARTITION_INTEGRITY"/>
      <xs:enumeration value="XM_HM_EV_MEM_PROTECTION"/>
      <xs:enumeration value="XM_HM_EV_OVERRUN"/>
      <xs:enumeration value="XM_HM_EV_SCHED_ERROR"/>
      <xs:enumeration value="XM_HM_EV_WATCHDOG_TIMER"/>
      <xs:enumeration value="XM_HM_EV_INCOMPATIBLE_INTERFACE"/>

#ifdef ia32
    <xs:enumeration value="XM_HM_EV_IA32_DIVIDE_EXCEPTION"/>
    <xs:enumeration value="XM_HM_EV_IA32_DEBUGGER_EXCEPTION"/>
    <xs:enumeration value="XM_HM_EV_IA32_NMI_EXCEPTION"/>
    <xs:enumeration value="XM_HM_EV_IA32_BREAKPOINT_EXCEPTION"/>
    <xs:enumeration value="XM_HM_EV_IA32_OVERFLOW_EXCEPTION"/>
    <xs:enumeration value="XM_HM_EV_IA32_BOUNDS_EXCEPTION"/>
    <xs:enumeration value="XM_HM_EV_IA32_INVALID_OPCODE"/>
    <xs:enumeration value="XM_HM_EV_IA32_COPROCESOR_UNAVAILABLE"/>
    <xs:enumeration value="XM_HM_EV_IA32_DOUBLE_FAULT"/>
    <xs:enumeration value="XM_HM_EV_IA32_COPROCESSOR_OVERRUN"/>
    <xs:enumeration value="XM_HM_EV_IA32_INVALID_TSS"/>
    <xs:enumeration value="XM_HM_EV_IA32_SEGMENT_NOT_PRESENT"/>
    <xs:enumeration value="XM_HM_EV_IA32_STACK_FAULT"/>
    <xs:enumeration value="XM_HM_EV_IA32_GENERAL_PROTECTION_FAULT"/>
    <xs:enumeration value="XM_HM_EV_IA32_PAGE_FAULT"/>
    <xs:enumeration value="XM_HM_EV_IA32_RESERVED"/>
    <xs:enumeration value="XM_HM_EV_IA32_MATH_FAULT"/>
    <xs:enumeration value="XM_HM_EV_IA32_ALIGNMENT_CHECK"/>
    <xs:enumeration value="XM_HM_EV_IA32_MACHINE_CHECK"/>
    <xs:enumeration value="XM_HM_EV_IA32_FLOATING_POINT_EXCEPTION"/>
#endif
      <!-- </track id="xml-list-hm-events" > -->
    </xs:restriction>
  </xs:simpleType>
	
  <xs:simpleType name="hmAction_t">
    <xs:restriction base="xs:string">
      <!-- <track id="xml-list-hm-actions" > -->
      <xs:enumeration value="XM_HM_AC_IGNORE"/>
      <xs:enumeration value="XM_HM_AC_SHUTDOWN"/>
      <xs:enumeration value="XM_HM_AC_COLD_RESET"/>
      <xs:enumeration value="XM_HM_AC_WARM_RESET"/>
      <xs:enumeration value="XM_HM_AC_SUSPEND"/>
      <xs:enumeration value="XM_HM_AC_HALT"/>
      <xs:enumeration value="XM_HM_AC_PROPAGATE"/>
      <!-- </track id="xml-list-hm-actions" > -->
    </xs:restriction>
  </xs:simpleType>
  
  <xs:simpleType name="portType_t">
    <xs:restriction base="xs:string">
      <xs:enumeration value="queuing"/>
      <xs:enumeration value="sampling"/>
    </xs:restriction>
  </xs:simpleType>
  
  <xs:simpleType name="direction_t">
    <xs:restriction base="xs:string">
      <xs:enumeration value="source"/>
      <xs:enumeration value="destination"/>
    </xs:restriction>
  </xs:simpleType>

  <xs:simpleType name="yntf_t">
    <xs:restriction base="xs:string">
      <xs:enumeration value="yes"/>
      <xs:enumeration value="no"/>
      <xs:enumeration value="true"/>
      <xs:enumeration value="false"/>
    </xs:restriction>
  </xs:simpleType>
  
  <!-- End Type -->

  <!-- Elements -->
  <!-- Hypervisor -->
  <xs:complexType name="hypervisor_element" mixed="false">
    <xs:all>
      <xs:element name="PhysicalMemoryAreas" type="memoryArea_element"/>
      <xs:element name="HwDescription" type="hwDescription_element" />
      <xs:element name="HealthMonitoring" type="healthMonitoring_element" minOccurs="0" />
      <xs:element name="Trace" type="trace_element" minOccurs="0" />
    </xs:all>
    <xs:attribute name="console" type="idString_t" use="optional"/>
    <xs:attribute name="healthMonitoringDevice" type="idString_t" use="optional"/>
    <xs:attribute name="loadPhysAddr" type="hex_t" use="required"/>
  </xs:complexType>

  <!-- Rsw -->
  <xs:complexType name="rsw_element">
    <xs:all>
      <xs:element name="PhysicalMemoryAreas" type="memoryArea_element"/>
    </xs:all>
    <xs:attribute name="entryPoint" type="hex_t" use="optional"/>
  </xs:complexType>
	
  <!-- Partition -->
  <xs:complexType name="partition_element" mixed="false">
    <xs:all>
      <xs:element name="PhysicalMemoryAreas" type="memoryArea_element"/>
      <xs:element name="TemporalRequirements" minOccurs="0">
	<xs:complexType>
	  <xs:attribute name="period" type="timeUnit_t" use="required"/>
	  <xs:attribute name="duration" type="timeUnit_t" use="required"/>
	</xs:complexType>	
      </xs:element>
      <xs:element name="HealthMonitoring" type="healthMonitoring_element" minOccurs="0" />
      <xs:element name="HwResources" type="hwResources_element" minOccurs="0" />
      <xs:element name="PortTable" type="partitionPorts_element" minOccurs="0" />
      <xs:element name="Trace" type="trace_element" minOccurs="0" />
    </xs:all>
    <xs:attribute name="id" type="partitionId_t" use="required"/>
    <xs:attribute name="name" type="idString_t" use="optional"/>
    <xs:attribute name="processor" type="id_t" use="required"/>
    <xs:attribute name="flags" type="partitionFlags_t" use="optional"/>
    <xs:attribute name="loadPhysAddr" type="hex_t" use="required"/>
    <xs:attribute name="headerOffset" type="hex_t" use="required"/>
    <xs:attribute name="fp" type="yntf_t" use="optional"  default="no"/>
    <xs:attribute name="imageId" type="hex_t" use="required"/>
    <xs:attribute name="console" type="idString_t" use="optional"/>
  </xs:complexType>

  <!-- Trace -->
  <xs:complexType name="trace_element">
    <xs:attribute name="device" type="idString_t" use="required"/>
    <xs:attribute name="bitmask" type="hex_t" use="required"/>
  </xs:complexType>

  <!-- Communication Ports -->
  <xs:complexType name="partitionPorts_element">
    <xs:sequence minOccurs="0" maxOccurs="unbounded">
      <xs:element name="Port">
	<xs:complexType>
	  <xs:attribute name="name" type="idString_t" use="required"/>
	  <xs:attribute name="direction" type="direction_t" use="required"/>
	  <xs:attribute name="type" type="portType_t" use="required"/>
	</xs:complexType>
      </xs:element>
    </xs:sequence>
  </xs:complexType> 
  
  <!-- Channels  -->
  <xs:complexType name="channels_element">
    <xs:sequence minOccurs="0" maxOccurs="unbounded">
      <xs:choice>
      <xs:element name="SamplingChannel">
	<xs:complexType>
	   <xs:sequence minOccurs="1">
	     <xs:element name="Source" type="ipcPort_element" />
	     <xs:sequence  minOccurs="1" maxOccurs="unbounded">
               <xs:element name="Destination" type="ipcPort_element"/>
             </xs:sequence>
	   </xs:sequence>           
	   <xs:attribute name="maxMessageLength" type="sizeUnit_t" use="required"/>
	   <xs:attribute name="validPeriod" type="timeUnit_t" use="optional"/>
	</xs:complexType>
      </xs:element>
      <xs:element name="QueuingChannel">
	<xs:complexType>
	  <xs:all minOccurs="1">
	    <xs:element name="Source" type="ipcPort_element" />
            <xs:element name="Destination" type="ipcPort_element"/>
	  </xs:all>
	  <xs:attribute name="maxMessageLength" type="sizeUnit_t" use="required"/>
	  <xs:attribute name="maxNoMessages" type="xs:positiveInteger" use="required"/>
	  <xs:attribute name="validPeriod" type="timeUnit_t" use="optional"/>
	</xs:complexType>
      </xs:element>
      </xs:choice>
    </xs:sequence>
  </xs:complexType>

  <!-- Devices  -->
  <xs:complexType name="devices_element">
    <xs:all>
#ifdef CONFIG_DEV_MEMBLOCK
       <xs:element name="MemoryBlockTable" type="memBlock_element" minOccurs="0" />
#endif

#ifdef CONFIG_DEV_PC_VGA
       <xs:element name="PcVga" type="pcVga_element" minOccurs="0" />
#endif
#ifdef CONFIG_DEV_PC_UART
       <xs:element name="PcUart" type="pcUart_element" minOccurs="0" />
#endif
    </xs:all>
  </xs:complexType>
	
 <!-- IPC Port -->
  <xs:complexType name="ipcPort_element">
    <xs:attribute name="partitionId" type="partitionId_t" use="required"/>
    <xs:attribute name="partitionName" type="idString_t" use="optional"/>
    <xs:attribute name="portName" type="idString_t" use="required"/>
  </xs:complexType>

  <!-- Sampling port -->
  
  <xs:complexType name="samplingPort_element">
    <xs:sequence>
      <xs:element name="Port" minOccurs="0" maxOccurs="unbounded">
	<xs:complexType>
	  <xs:attribute name="name" type="idString_t" use="required"/>
	  <xs:attribute name="direction" type="direction_t" use="required"/>
	  <!--<xs:attribute name="maxMessageLength" type="sizeUnit_t"
	  use="required"/>-->
	</xs:complexType>
      </xs:element>
    </xs:sequence>
  </xs:complexType>
  
  <!-- Queueing port -->
  
  <xs:complexType name="queuingPort_element">
      <xs:sequence>
	<xs:element name="Port" minOccurs="0" maxOccurs="unbounded">
	  <xs:complexType>
	    <xs:attribute name="name" type="idString_t" use="required"/>
	    <xs:attribute name="direction" type="direction_t" use="required"/>
	    <!-- <xs:attribute name="maxMessageLength"
	    type="sizeUnit_t" use="required"/>
		 <xs:attribute name="maxNoMessages"
	    type="xs:positiveInteger" use="required"/>-->

	  </xs:complexType>
	</xs:element>
      </xs:sequence>
  </xs:complexType>
 
 <!-- Hw Description -->
  <xs:complexType name="hwDescription_element">
    <xs:sequence>
    <xs:sequence minOccurs="1" maxOccurs="# CONFIG_NO_CPUS #">
      <xs:element name="Processor" type="processor_element" />
    </xs:sequence>
    <xs:element name="MemoryLayout" type="memoryLayout_element"/>
    </xs:sequence>
  </xs:complexType>
 
  <!-- Processor -->
  <xs:complexType name="processor_element">
    <xs:sequence minOccurs="1" maxOccurs="1">
      <xs:element name="Sched" type="sched_element"/>
    </xs:sequence>
    <xs:attribute name="id" type="processorId_t" use="required"/>
    <xs:attribute name="frequency" type="freqUnit_t" use="optional"/>
    <xs:attribute name="features" type="processorFeatures_t" use="optional" />
  </xs:complexType>

  <!-- HwResource -->
  <xs:complexType name="hwResources_element">
    <xs:all>
      <xs:element name="IoPorts" type="ioPorts_element" minOccurs="0" />
      <xs:element name="Interrupts" type="interrupts_element" minOccurs="0" />
    </xs:all>
  </xs:complexType>

  <!-- Io Ports -->
  <xs:complexType name="ioPorts_element">
    <xs:sequence minOccurs="0" maxOccurs="unbounded">
      <xs:choice>
	<xs:element name="Range">
          <xs:complexType>
	    <xs:attribute name="base" type="hex_t" use="required"/>
	    <xs:attribute name="noPorts" type="xs:positiveInteger" use="required"/>
          </xs:complexType>
	</xs:element>
	<xs:element name="Restricted">
	  <xs:complexType>
	    <xs:attribute name="address" type="hex_t" use="required"/>
	    <xs:attribute name="mask" type="hex_t"   use="optional"/>
	  </xs:complexType>
	</xs:element>
      </xs:choice>      
    </xs:sequence>
  </xs:complexType>
  
  <!-- Hw Interrupts -->
  <xs:complexType name="interrupts_element">
	 <xs:attribute name="line" type="hwIrqLine_t" use="required"/>
  </xs:complexType>
       
  <!-- Sched -->
  <xs:complexType name="sched_element">
    <xs:choice>
      <xs:element name="CyclicPlanTable" type="cyclicPlan_element"/>
      <!-- <xs:element name="FixPriority">
	<xs:complexType>
	  <xs:attribute name="priority" type="id_t" use="required"/>
	</xs:complexType>
      </xs:element> -->
    </xs:choice>
  </xs:complexType>

  <!-- CyclicPlan -->
  <xs:complexType name="cyclicPlan_element">
    <xs:sequence minOccurs="1" maxOccurs="unbounded">
      <xs:element name="Plan" type="plan_element" />
    </xs:sequence>
  </xs:complexType>

  <!-- Plan -->
  <xs:complexType name="plan_element">
    <xs:sequence minOccurs="1" maxOccurs="unbounded">
      <xs:element name="Slot">
	<xs:complexType>
 	  <xs:attribute name="id" type="id_t" use="required"/>
          <xs:attribute name="start" type="timeUnit_t" use="required"/>
          <xs:attribute name="duration" type="timeUnit_t" use="required"/>
          <xs:attribute name="partitionId" type="partitionId_t" use="required"/>
        </xs:complexType>
      </xs:element>
    </xs:sequence>
    <xs:attribute name="id" type="id_t" use="required"/>
    <xs:attribute name="majorFrame" type="timeUnit_t" use="required"/>
  </xs:complexType>

  <!-- Health Monitoring -->
  <xs:complexType name="healthMonitoring_element">
    <xs:sequence  minOccurs="1" maxOccurs="unbounded">
      <xs:element name="Event">
        <xs:complexType>
	  <xs:attribute name="name" type="hmString_t" use="optional"/>
	  <xs:attribute name="action" type="hmAction_t" use="required"/>
	  <xs:attribute name="log" type="yntf_t" use="required"/>
	</xs:complexType>
      </xs:element>
    </xs:sequence>
  </xs:complexType>

  <!-- Memory Layout -->
  <xs:complexType name="memoryLayout_element">
    <xs:sequence minOccurs="1" maxOccurs="unbounded">
      <xs:element name="Region">
        <xs:complexType>
 	  <xs:attribute name="type" type="memRegion_t" use="required"/>
          <xs:attribute name="start" type="hex_t" use="required"/>
          <xs:attribute name="size" type="sizeUnit_t" use="required"/>
        </xs:complexType>
      </xs:element>
    </xs:sequence>
  </xs:complexType>
  
 <!-- Memory Area -->
  <xs:complexType name="memoryArea_element">
    <xs:sequence minOccurs="1" maxOccurs="unbounded">
      <xs:element name="Area">
        <xs:complexType>
          <xs:attribute name="start" type="hex_t" use="required"/>
          <xs:attribute name="size" type="sizeUnit_t" use="required"/>
          <xs:attribute name="flags" type="memAreaFlags_t" use="optional" />
        </xs:complexType>
      </xs:element>
    </xs:sequence>
  </xs:complexType>
	
  <!-- Root Element -->
  <xs:element name="SystemDescription">
    <xs:complexType>
      <xs:all>
	<xs:element name="XMHypervisor" type="hypervisor_element"/>
	<xs:element name="ResidentSw" type="rsw_element" minOccurs="0" />
	<xs:element name="PartitionTable">
	<xs:complexType>
          <xs:sequence maxOccurs="unbounded">	 
	    <xs:element name="Partition" type="partition_element" />
	  </xs:sequence>
	</xs:complexType>
	</xs:element>
      <xs:element name="Channels" type="channels_element" minOccurs="0" />
      <xs:element name="Devices" type="devices_element" minOccurs="0" />
      </xs:all>
      <xs:attribute name="version" type="version_t" use="required"/>
      <xs:attribute name="name" type="idString_t" use="required"/>    
      </xs:complexType>
   </xs:element>
   <!-- End Root Element -->
   <!-- Elements -->
</xs:schema>

